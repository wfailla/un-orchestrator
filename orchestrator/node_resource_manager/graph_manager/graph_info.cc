#include "graph_info.h"

GraphInfo::GraphInfo() :
	controller(NULL), lsi(NULL), computeController(NULL)//, graph(NULL)
{

}

GraphInfo::~GraphInfo()
{
}

void GraphInfo::setController(Controller *controller)
{
	this->controller = controller;
}

void GraphInfo::setLSI(LSI *lsi)
{
	this->lsi = lsi;
}

void GraphInfo::setComputeController(ComputeController *computeController)
{
	this->computeController = computeController;
}

#ifdef ENABLE_UNIFY_MONITORING_CONTROLLER
void GraphInfo::setMonitoringController(MonitoringController *monitoringController)
{
	this->monitoringController = monitoringController;
}
#endif

void GraphInfo::setGraph(highlevel::Graph *graph)
{
	this->graph = graph;
}

Controller *GraphInfo::getController()
{
	return controller;
}

LSI *GraphInfo::getLSI()
{
	return lsi;
}

highlevel::Graph *GraphInfo::getGraph()
{
	return graph;
}

ComputeController *GraphInfo::getComputeController()
{
	return computeController;
}

#ifdef ENABLE_UNIFY_MONITORING_CONTROLLER
MonitoringController *GraphInfo::getMonitoringController()
{
	return monitoringController;
}
#endif
