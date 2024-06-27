#include <controller/rio_Controller.h>
#include <rio.h>
#include <gfx/rio_Camera.h>

void useFlyCam(rio::LookAtCamera *camera, rio::Controller *controller);
void useTargetPoint(rio::LookAtCamera *camera, rio::Controller *controller, rio::Vector3f targetPoint, float camDistance);