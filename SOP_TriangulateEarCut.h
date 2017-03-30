#pragma once

#include <SOP/SOP_API.h>
#include <SOP/SOP_Node.h>


class GU_Detail;
class GA_Primitive;


namespace EarCutAxis
{
    enum Enum
    {
        AxisXY = 0,
        AxisXZ,
        AxisYZ
    };
}


class SOP_TriangulateEarCut : public SOP_Node
{
    public:

        static OP_Node* myConstructor(OP_Network* network, const char* name, OP_Operator* op);
        static PRM_Template myTemplateList[];

    protected:

        SOP_TriangulateEarCut(OP_Network* network, const char* name, OP_Operator* op);
        virtual ~SOP_TriangulateEarCut();

    protected:

        //! Retrieve ear cut axis.
        EarCutAxis::Enum getParamEarCutAxis(fpreal t) const;

        //! Retrieve point group name which we are using.
        void getParamInputPointGroup(UT_String& input_point_group, fpreal t) const;

        //! Whether to keep any incoming primitives.
        bool getParamKeepIncomingPrimitives(fpreal t) const;

    protected:

        //! Given a point group, look it up, return null if not found.
        GA_PointGroup* findPointGroup(const UT_String& group_name) const;

    protected:

        virtual OP_ERROR cookMySop(OP_Context& context);
        virtual const char* inputLabel(unsigned int idx) const;
};
