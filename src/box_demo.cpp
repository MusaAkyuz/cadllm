#include "box_demo.h"

#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <GProp_GProps.hxx>
#include <TopoDS_Shape.hxx>

double demoBoxVolume()
{
    TopoDS_Shape box = BRepPrimAPI_MakeBox(10.0, 20.0, 30.0).Shape();

    GProp_GProps props;
    BRepGProp::VolumeProperties(box, props);

    return props.Mass();
}
