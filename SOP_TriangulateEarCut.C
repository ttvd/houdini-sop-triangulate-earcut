#include "SOP_TriangulateEarCut.h"

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>

#include <earcut.hpp>
#include <vector>


#define SOP_TRIANGULATE_EAR_CUT_NAME "triangulateearcut"
#define SOP_TRIANGULATE_EAR_CUT_DESCRIPTION "TriangulateEarCut"

#define SOP_TRIANGULATE_EAR_CUT_AXIS "earcut_axis"
#define SOP_TRIANGULATE_EAR_CUT_INPUT_POINT_GROUP "earcut_input_point_group"
#define SOP_TRIANGULATE_EAR_CUT_KEEP_INCOMING_PRIMITIVES "earcut_keep_incoming_prims"


struct EarCutHoudiniPoint
{
    EarCutHoudiniPoint() :
        point_offset(GA_INVALID_OFFSET),
        point_position(0.0f, 0.0f, 0.0f),
        ear_cut_axis(EarCutAxis::AxisXY)
    {
    }

    EarCutHoudiniPoint(GA_Offset in_point_offset, const UT_Vector3& in_point_position,
        EarCutAxis::Enum in_ear_cut_axis) :
        point_offset(in_point_offset),
        point_position(in_point_position),
        ear_cut_axis(in_ear_cut_axis)
    {
    }

    EarCutHoudiniPoint(const EarCutHoudiniPoint& ear_cut) :
        point_offset(ear_cut.point_offset),
        point_position(ear_cut.point_position),
        ear_cut_axis(ear_cut.ear_cut_axis)
    {
    }

    GA_Offset point_offset;
    UT_Vector3 point_position;
    EarCutAxis::Enum ear_cut_axis;
};


namespace mapbox
{
    namespace util
    {
        template <>
        struct nth<0, EarCutHoudiniPoint>
        {
            inline static float get(const EarCutHoudiniPoint& ear_cut_point)
            {
                const UT_Vector3& pos = ear_cut_point.point_position;

                switch(ear_cut_point.ear_cut_axis)
                {
                    default:
                    case EarCutAxis::AxisXY:
                    case EarCutAxis::AxisXZ:
                    {
                        return pos.x();
                    }

                    case EarCutAxis::AxisYZ:
                    {
                        return pos.y();
                    }
                }
            };
        };

        template <>
        struct nth<1, EarCutHoudiniPoint>
        {
            inline static float get(const EarCutHoudiniPoint& ear_cut_point)
            {
                const UT_Vector3& pos = ear_cut_point.point_position;

                switch(ear_cut_point.ear_cut_axis)
                {
                    default:
                    case EarCutAxis::AxisXY:
                    {
                        return pos.y();
                    }

                    case EarCutAxis::AxisXZ:
                    case EarCutAxis::AxisYZ:
                    {
                        return pos.z();
                    }
                }
            };
        };
    }
}


static PRM_Name s_name_earcut_axis(SOP_TRIANGULATE_EAR_CUT_AXIS, "EarCut Axis");
static PRM_Name s_name_earcut_axis_types[] =
{
    PRM_Name("xy", "XY"),
    PRM_Name("xz", "XZ"),
    PRM_Name("yz", "YZ"),
    PRM_Name(0),
};
static PRM_Name s_name_input_point_group(SOP_TRIANGULATE_EAR_CUT_INPUT_POINT_GROUP, "Input Point Group");
static PRM_Name s_name_keep_incoming_prims(SOP_TRIANGULATE_EAR_CUT_KEEP_INCOMING_PRIMITIVES,
    "Keep Incoming Primitives");

static PRM_ChoiceList s_choicelist_earcut_axis(PRM_CHOICELIST_SINGLE, s_name_earcut_axis_types);
static PRM_Default s_default_keep_incoming_prims(true);
static PRM_Default s_default_earcut_axis((int) EarCutAxis::AxisXZ);

PRM_Template
SOP_TriangulateEarCut::myTemplateList[] =
{
    PRM_Template(PRM_STRING, 1, &s_name_input_point_group, 0, &SOP_Node::pointGroupMenu, 0, 0,
        SOP_Node::getGroupSelectButton(GA_GROUP_INVALID, PRMgroupTypeName.getToken())),
    PRM_Template(PRM_TOGGLE, 1, &s_name_keep_incoming_prims,
        &s_default_keep_incoming_prims),
    PRM_Template(PRM_ORD, 1, &s_name_earcut_axis, &s_default_earcut_axis, &s_choicelist_earcut_axis),
    PRM_Template()
};


OP_Node*
SOP_TriangulateEarCut::myConstructor(OP_Network* network, const char* name, OP_Operator* op)
{
    return new SOP_TriangulateEarCut(network, name, op);
}


SOP_TriangulateEarCut::SOP_TriangulateEarCut(OP_Network* network, const char* name, OP_Operator* op) :
    SOP_Node(network, name, op)
{

}


SOP_TriangulateEarCut::~SOP_TriangulateEarCut()
{

}


OP_ERROR
SOP_TriangulateEarCut::cookMySop(OP_Context& context)
{
    if(error() >= UT_ERROR_ABORT)
    {
        return error();
    }

    // Get current execution time for parameter evaluation.
    fpreal t = context.getTime();

    // Get parameter values.
    EarCutAxis::Enum ear_cut_axis = getParamEarCutAxis(t);
    bool keep_incoming_primitives = getParamKeepIncomingPrimitives(t);

    // Get point group.
    GA_PointGroup* point_group = nullptr;

    {
        UT_String group_name;
        getParamInputPointGroup(group_name, t);
        point_group = findPointGroup(group_name);
    }

    // Lock inputs to avoid race.
    if(lockInputs(context) >= UT_ERROR_ABORT)
    {
        return error();
    }

    // Duplicate the source.
    duplicateSource(0, context);

    // Remove primitives, if requested.
    if(!keep_incoming_primitives)
    {
        GA_Primitive* prim = nullptr;
        GA_Primitive* prim_next = nullptr;

        GA_FOR_SAFE_PRIMITIVES(gdp, prim, prim_next)
        {
            GA_Offset prim_offset = prim->getMapOffset();
            gdp->destroyPrimitiveOffset(prim_offset, false);
        }
    }

    // Collect all points.
    std::vector<std::vector<EarCutHoudiniPoint> > houdini_points;
    std::vector<EarCutHoudiniPoint> gdp_points;

    {
        if(point_group)
        {
            GA_Offset point_offset = GA_INVALID_OFFSET;
            GA_FOR_ALL_GROUP_PTOFF(gdp, point_group, point_offset)
            {
                const UT_Vector3& point_position = gdp->getPos3(point_offset);
                EarCutHoudiniPoint ear_cut_point(point_offset, point_position, ear_cut_axis);
                gdp_points.push_back(ear_cut_point);
            }

            houdini_points.push_back(gdp_points);
        }
        else
        {
            GA_Offset point_offset = GA_INVALID_OFFSET;
            GA_FOR_ALL_PTOFF(gdp, point_offset)
            {
                const UT_Vector3& point_position = gdp->getPos3(point_offset);
                EarCutHoudiniPoint ear_cut_point(point_offset, point_position, ear_cut_axis);
                gdp_points.push_back(ear_cut_point);
            }

            houdini_points.push_back(gdp_points);
        }
    }

    // Compute primitive indices.
    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(houdini_points);

    if(!indices.size())
    {
        UT_WorkBuffer buf;
        buf.sprintf("Triangulate Ear Cut: No indices computed.");
        addError(SOP_MESSAGE, buf.buffer());
        unlockInputs();

        return error(context);
    }

    for(int idx = 0; idx < indices.size(); idx += 3)
    {
        const EarCutHoudiniPoint& houdini_point_0 = gdp_points[indices[idx + 0]];
        const EarCutHoudiniPoint& houdini_point_1 = gdp_points[indices[idx + 1]];
        const EarCutHoudiniPoint& houdini_point_2 = gdp_points[indices[idx + 2]];

        GU_PrimPoly* poly = GU_PrimPoly::build(gdp, 0, GU_POLY_CLOSED);
        poly->appendVertex(houdini_point_0.point_offset);
        poly->appendVertex(houdini_point_1.point_offset);
        poly->appendVertex(houdini_point_2.point_offset);
        poly->close();
    }

    gdp->getAttributes().bumpAllDataIds(GA_ATTRIB_PRIMITIVE);
    gdp->getPrimitiveList().bumpDataId();

    unlockInputs();
    return error(context);
}


EarCutAxis::Enum
SOP_TriangulateEarCut::getParamEarCutAxis(fpreal t) const
{
    return (EarCutAxis::Enum) evalInt(SOP_TRIANGULATE_EAR_CUT_AXIS, 0, t);
}


void
SOP_TriangulateEarCut::getParamInputPointGroup(UT_String& input_point_group, fpreal t) const
{
    evalString(input_point_group, SOP_TRIANGULATE_EAR_CUT_INPUT_POINT_GROUP, 0, t);
}


bool
SOP_TriangulateEarCut::getParamKeepIncomingPrimitives(fpreal t) const
{
    return evalInt(SOP_TRIANGULATE_EAR_CUT_KEEP_INCOMING_PRIMITIVES, 0, t) != 0;
}


GA_PointGroup*
SOP_TriangulateEarCut::findPointGroup(const UT_String& group_name) const
{
    if(!gdp)
    {
        return nullptr;
    }

    if(!group_name.isstring() || !group_name.isValidVariableName())
    {
        return nullptr;
    }

    GA_PointGroup* lookup_group = gdp->findPointGroup(group_name);
    return lookup_group;
}


const char*
SOP_TriangulateEarCut::inputLabel(unsigned int idx) const
{
    return "Input Points";
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator(SOP_TRIANGULATE_EAR_CUT_NAME, SOP_TRIANGULATE_EAR_CUT_DESCRIPTION,
        SOP_TriangulateEarCut::myConstructor, SOP_TriangulateEarCut::myTemplateList, 1, 1, 0, OP_FLAG_GENERATOR));
}

