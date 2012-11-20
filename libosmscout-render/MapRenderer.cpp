/*
    libosmscout-render

    Copyright (C) 2012, Preet Desai

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "MapRenderer.h"


namespace osmsrender
{

MapRenderer::MapRenderer()
{}

MapRenderer::~MapRenderer()
{}

// ========================================================================== //
// ========================================================================== //

void MapRenderer::AddDataSet(DataSetOSM * dataSet)
{
    m_listDataSets.push_back(dataSet);
    rebuildAllData();
}

void MapRenderer::RemoveDataSet(DataSetOSM * dataSet)
{
    std::vector<DataSet*>::iterator dsIt;
    for(dsIt = m_listDataSets.begin();
        dsIt != m_listDataSets.end(); ++dsIt)
    {
        if(static_cast<DataSet*>(dataSet) == (*dsIt))  // TODO: check
        {   break;   }
    }
    m_listDataSets.erase(dsIt);
    rebuildAllData();
}

void MapRenderer::AddDataSet(DataSetOSMCoast * dataSet)
{
    m_listDataSets.push_back(dataSet);
    rebuildAllData();
}

void MapRenderer::RemoveDataSet(DataSetOSMCoast * dataSet)
{
    std::vector<DataSet*>::iterator dsIt;
    for(dsIt = m_listDataSets.begin();
        dsIt != m_listDataSets.end(); ++dsIt)
    {
        if(static_cast<DataSet*>(dataSet) == (*dsIt))  // TODO: check
        {   break;   }
    }
    m_listDataSets.erase(dsIt);
    rebuildAllData();
}

void MapRenderer::AddDataSet(DataSetTemp *dataSet)
{
    m_listDataSets.push_back(dataSet);
    rebuildAllData();
}

void MapRenderer::RemoveDataSet(DataSetTemp *dataSet)
{
    std::vector<DataSet*>::iterator dsIt;
    for(dsIt = m_listDataSets.begin();
        dsIt != m_listDataSets.end(); ++dsIt)
    {
        if(static_cast<DataSet*>(dataSet) == (*dsIt))  // TODO: check
        {   break;   }
    }
    m_listDataSets.erase(dsIt);
    rebuildAllData();
}

// ========================================================================== //
// ========================================================================== //

void MapRenderer::SetRenderStyle(const std::string &stylePath)
{
    m_stylePath = stylePath;

    rebuildAllData();
    OSRDEBUG << "INFO: Set New Style: " << m_stylePath;
}

void MapRenderer::GetDebugLog(std::vector<std::string> &listDebugMessages)
{
    for(size_t i=0; i < m_listMessages.size(); i++)
    {   listDebugMessages.push_back(m_listMessages.at(i));   }
}

// ========================================================================== //
// ========================================================================== //

void MapRenderer::InitializeScene()
{
    if(m_listDataSets.size() < 1)
    {   OSRDEBUG << "ERROR: No available data sets!";   return;   }

    // set camera / update scene
    double minLat,minLon,maxLat,maxLon;
    m_listDataSets[0]->GetBoundingBox(minLat,minLon,maxLat,maxLon);
    PointLLA camLLA((minLat+maxLat)/2,(minLon+maxLon)/2,500.0);

    SetCamera(camLLA,30.0,1.67);
}

void MapRenderer::InitializeScene(const PointLLA &camLLA,
                                  double fovy, double aspectRatio)
{
    if(m_listDataSets.size() < 1)
    {   OSRDEBUG << "ERROR: No available data sets!";   return;   }

    // set camera
    SetCamera(camLLA,fovy,aspectRatio);
}

void MapRenderer::SetCamera(const PointLLA &camLLA,
                            double fovy, double aspectRatio)
{
    Vec3 camNorth,camEast,camDown;
    calcECEFNorthEastDown(camLLA,camNorth,camEast,camDown);

    m_camera.LLA = camLLA;
    m_camera.eye = convLLAToECEF(camLLA);
    m_camera.viewPt = m_camera.eye+camDown.Normalized().ScaledBy(camLLA.alt);
    m_camera.up = camNorth;
    m_camera.fovY = fovy;
    m_camera.aspectRatio = aspectRatio;

    if(!calcCamViewExtents(m_camera))
    {   OSRDEBUG << "WARN: Could not calculate view extents";   }
    else
    {   updateSceneBasedOnCamera();   }
}

void MapRenderer::UpdateCameraLookAt(const Vec3 &eye,
                                     const Vec3 &viewPt,
                                     const Vec3 &up)
{
    m_camera.eye = eye;
    m_camera.viewPt = viewPt;
    m_camera.up = up;
    m_camera.LLA = convECEFToLLA(m_camera.eye);

    // update scene if required
    if(!calcCamViewExtents(m_camera))
    {   OSRDEBUG << "WARN: Could not calculate view extents";   }
    else
    {   updateSceneBasedOnCamera();   }
}

void MapRenderer::UpdateSceneContents(const DataSet *dataSet)
{
    // check if MapRenderer contains the DataSet
    std::vector<DataSet*> listDataSets;
    for(size_t i=0; i < m_listDataSets.size(); i++)   {
        if(m_listDataSets[i] == dataSet)   {
            listDataSets.push_back(m_listDataSets[i]);
        }
    }
    updateSceneContents(listDataSets);
}

void MapRenderer::UpdateSceneContentsAll()
{
    updateSceneContents(m_listDataSets);
}

Camera const * MapRenderer::GetCamera()
{   return &m_camera;   }

// ========================================================================== //
// ========================================================================== //

void MapRenderer::rebuildAllData()
{
    if((m_listDataSets.size() < 1) || (m_stylePath.empty()))
    {   return;   }

    // clear implemented scene
    removeAllFromScene();

    OSRDEBUG << "===================================";
    std::vector<DataSet*>::iterator dsIt;
    std::vector<DataSet const *> listKDataSetPtrs;
    for(dsIt = m_listDataSets.begin();
        dsIt != m_listDataSets.end(); ++dsIt)
    {
        DataSet * dataSet = (*dsIt);

        // clear existing render data
        dataSet->listNodeData.clear();
        dataSet->listWayData.clear();
        dataSet->listAreaData.clear();
        dataSet->listRelWayData.clear();
        dataSet->listRelAreaData.clear();
        dataSet->listSharedNodes.clear();

        // remove old style data
        for(size_t i=0; i < dataSet->listStyleConfigs.size(); i++)
        {   delete dataSet->listStyleConfigs[i];   }
        dataSet->listStyleConfigs.clear();

        // add new style data
        bool opOk = false;
        RenderStyleReader styleReader(m_stylePath,
            dataSet->GetTypeConfig(),
            dataSet->listStyleConfigs,opOk);

        if(!opOk)   {
            OSRDEBUG << "ERROR: Could not set style info";
            return;
        }

        size_t numLods = dataSet->listStyleConfigs.size();
        dataSet->listNodeData.resize(numLods);
        dataSet->listWayData.resize(numLods);
        dataSet->listAreaData.resize(numLods);
        dataSet->listRelWayData.resize(numLods);
        dataSet->listRelAreaData.resize(numLods);

        for(size_t i=0; i < numLods; i++)   {
            // todo: empirically determine reserve count
            //       (or if we even need them at all)
            dataSet->listNodeData[i].reserve(50);
            dataSet->listWayData[i].reserve(150);
            dataSet->listAreaData[i].reserve(100);
            dataSet->listRelWayData[i].reserve(5);
            dataSet->listRelAreaData[i].reserve(25);
        }
        dataSet->listSharedNodes.reserve(2000);

        // save list for virtual implementation
        listKDataSetPtrs.push_back(dataSet);
    }

    rebuildStyleData(listKDataSetPtrs);
    updateSceneContents(m_listDataSets);

    OSRDEBUG << "===================================";
}

void MapRenderer::updateSceneContents(std::vector<DataSet*> &listDataSets)
{
    if(listDataSets.size() < 1)
    {   return;   }

    // calculate the minimum and maximum distance to
    // m_camera.eye within the available lat/lon bounds
    double minViewDist,maxViewDist;
    this->calcCamViewDistances(minViewDist,maxViewDist);

    OSRDEBUG << "### Camera Min View Dist: " << minViewDist;
    OSRDEBUG << "### Camera Max View Dist: " << maxViewDist;

    // use the min and max distance between m_camera.eye
    // and the view bounds to set active LOD ranges

    // (range data is common amongst DataSet style configs)
    size_t numLodRanges = listDataSets[0]->listStyleConfigs.size();
    std::vector<bool> listLODRangesActive(numLodRanges);
    std::vector<std::pair<double,double> > listLODRanges(numLodRanges);

    for(size_t i=0; i < numLodRanges; i++)
    {
        std::pair<double,double> lodRange;
        lodRange.first = listDataSets[0]->listStyleConfigs[i]->GetMinDistance();
        lodRange.second = listDataSets[0]->listStyleConfigs[i]->GetMaxDistance();
        listLODRanges[i] = lodRange;

        // if the min-max distance range overlaps with
        // lodRange, set the range as active
        if(lodRange.second < minViewDist || lodRange.first > maxViewDist)
        {   listLODRangesActive[i] = false;   }
        else
        {   listLODRangesActive[i] = true;   }
    }

    // check if at least one valid style
    bool hasValidStyle = false;
    for(size_t i=0; i < listLODRangesActive.size(); i++)
    {
        if(listLODRangesActive[i])
        {   hasValidStyle = true;   break;   }
    }

    if(!hasValidStyle)
    {
        OSRDEBUG << "WARN: No valid style data found";

        // hide all data
//        this->toggleSceneVisibility(false);

        // (experimental) -- prefer to HIDE instead of delete
        // remove all scene data if no style data is available
//        std::vector<DataSet*>::iterator dsIt;
//        for(dsIt = m_listDataSets.begin();
//            dsIt != m_listDataSets.end(); ++dsIt)
//        {
//            DataSet * dataSet = (*dsIt);

//            // clear existing render data
//            dataSet->listNodeData.clear();
//            dataSet->listWayData.clear();
//            dataSet->listAreaData.clear();
//            dataSet->listRelWayData.clear();
//            dataSet->listRelAreaData.clear();
//            dataSet->listSharedNodes.clear();

//            size_t numLods = dataSet->listStyleConfigs.size();
//            dataSet->listNodeData.resize(numLods);
//            dataSet->listWayData.resize(numLods);
//            dataSet->listAreaData.resize(numLods);
//            dataSet->listRelWayData.resize(numLods);
//            dataSet->listRelAreaData.resize(numLods);
//        }
//        this->removeAllFromScene();
//        this->doneUpdatingAreas();
//        this->doneUpdatingRelAreas();

//        // update current data extents
//        m_data_exTL = m_camera.exTL;
//        m_data_exTR = m_camera.exTR;
//        m_data_exBL = m_camera.exBL;
//        m_data_exBR = m_camera.exBR;

        return;
    }
    else
    {   toggleSceneVisibility(true);   }

    PointLLA camLLA = convECEFToLLA(m_camera.eye);
    size_t num_lod_ranges = listLODRanges.size();

    // for specified DataSets
    size_t dme = 0; // todo delete me
    std::vector<DataSet*>::iterator dsIt;
    for(dsIt = listDataSets.begin();
        dsIt != listDataSets.end(); ++dsIt)
    {
        dme++;
        // get style configs belonging to this DataSet
        DataSet * dataSet = (*dsIt);
        std::vector<RenderStyleConfig*> &listStyleConfigs =
                dataSet->listStyleConfigs;

        // create lists to hold database results by lod
        ListNodeRefsByLod    listNodeRefsByLod(num_lod_ranges);
        ListWayRefsByLod     listWayRefsByLod(num_lod_ranges);
        ListAreaRefsByLod    listAreaRefsByLod(num_lod_ranges);
        ListRelWayRefsByLod  listRelWayRefsByLod(num_lod_ranges);
        ListRelAreaRefsByLod listRelAreaRefsByLod(num_lod_ranges);

        // create sets to hold database results for all lods
        TYPE_UNORDERED_SET<osmscout::Id> setNodesAllLods(300);
        TYPE_UNORDERED_SET<osmscout::Id> setWaysAllLods(600);
        TYPE_UNORDERED_SET<osmscout::Id> setAreasAllLods(300);
        TYPE_UNORDERED_SET<osmscout::Id> setRelWaysAllLods(50);
        TYPE_UNORDERED_SET<osmscout::Id> setRelAreasAllLods(100);

        for(size_t i=0; i < num_lod_ranges; i++)
        {
            if(listLODRangesActive[i])
            {
                // create bounds for active LOD range
                Vec3 rangeTL,rangeTR,rangeBR,rangeBL;
                calcDistBoundingBox(camLLA,listLODRanges[i].second,
                                    rangeTL,rangeTR,rangeBR,rangeBL);

                std::vector<Vec3> listVxB1(4);
                listVxB1[0] = m_camera.exTL;
                listVxB1[1] = m_camera.exTR;
                listVxB1[2] = m_camera.exBR;
                listVxB1[3] = m_camera.exBL;

                std::vector<Vec3> listVxB2(4);
                listVxB2[0] = rangeTL;
                listVxB2[1] = rangeTR;
                listVxB2[2] = rangeBR;
                listVxB2[3] = rangeBL;

                // find overlap between camera extents and LOD range
                std::vector<Vec3> listVxROI; Vec3 vxROICentroid;
                if(!calcBoundsIntersection(listVxB1,listVxB2,listVxROI,vxROICentroid))
                {   OSRDEBUG << "WARN: Could not find LOD Overlap";  return;   }

                if(listVxROI.size() < 3)
                {   OSRDEBUG << "WARN: Invalid LOD Overlap";  return;   }

                // get minimum enclosing bounds in lon/lat
                // note: for the point within the bounds, we use
                // the centroid of a triangle from its poly

                std::vector<GeoBounds> listQueries;
                calcEnclosingGeoBounds(listVxROI,listQueries);

                // get objects from database
                osmscout::TypeSet typeSet;
                listStyleConfigs[i]->GetActiveTypes(typeSet);

                std::vector<osmscout::NodeRef>        listNodeRefs;
                std::vector<osmscout::WayRef>         listWayRefs;
                std::vector<osmscout::WayRef>         listAreaRefs;
                std::vector<osmscout::RelationRef>    listRelWayRefs;
                std::vector<osmscout::RelationRef>    listRelAreaRefs;

                if(dataSet->GetObjects(listQueries,
                                       typeSet,
                                       listNodeRefs,
                                       listWayRefs,
                                       listAreaRefs,
                                       listRelWayRefs,
                                       listRelAreaRefs))
                {
                    // we retrieve objects from a high LOD (close up)
                    // to a lower LOD (further away)

                    // sets are used to store database results from
                    // previous LODs to ensure that no duplicate entries
                    // exist between LODs (a single object should only
                    // be displayed once in the scene)

                    // note: since some types can be shared in the definition file,
                    // we need to exclusively keep track of which sets of nodes, ways
                    // areas should be kept -- so even if the db query returns certain
                    // primitives, we only use them if they are explicitly specified

                    // [nodes]
                    std::vector<osmscout::NodeRef>::iterator nodeIt;
                    for(nodeIt = listNodeRefs.begin();
                        nodeIt != listNodeRefs.end(); ++nodeIt)
                    {
                        if(listStyleConfigs[i]->GetNodeTypeIsValid((*nodeIt)->GetType()))
                        {
                            // note: libosmscout returns a lot of nodes well beyond the
                            // specified bounds, so we check if nodes are in our ROI

                            double myLat = (*nodeIt)->GetLat();                   // TODO FIXME
                            double myLon = (*nodeIt)->GetLon();                   // TODO FIXME

                            for(size_t b=0; b < listQueries.size(); b++)
                            {
                                if(myLat > listQueries[b].minLat &&
                                   myLat < listQueries[b].maxLat &&
                                   myLon > listQueries[b].minLon &&
                                   myLon < listQueries[b].maxLon)
                                {
                                    if(setNodesAllLods.insert((*nodeIt)->GetId()).second)   {
                                        listNodeRefsByLod[i].insert(std::make_pair((*nodeIt)->GetId(),*nodeIt));
                                    }
                                }
                            }
                        }
                    }

                    // [ways]
                    std::vector<osmscout::WayRef>::iterator wayIt;
                    for(wayIt = listWayRefs.begin();
                        wayIt != listWayRefs.end(); ++wayIt)
                    {
                        if(listStyleConfigs[i]->GetWayTypeIsValid((*wayIt)->GetType()))
                        {
                            if(setWaysAllLods.insert((*wayIt)->GetId()).second)   {
                                listWayRefsByLod[i].insert(std::make_pair((*wayIt)->GetId(),*wayIt));
                            }
                        }
                    }

                    // [areas]
                    std::vector<osmscout::WayRef>::iterator areaIt;
                    for(areaIt = listAreaRefs.begin();
                        areaIt != listAreaRefs.end(); ++areaIt)
                    {
                        if(listStyleConfigs[i]->GetAreaTypeIsValid((*areaIt)->GetType()))
                        {
                            if(setAreasAllLods.insert((*areaIt)->GetId()).second)   {
                                listAreaRefsByLod[i].insert(std::make_pair((*areaIt)->GetId(),*areaIt));
                            }
                        }
                    }

                    // [relation ways]
                    // (todo)

                    // [relation areas]
                    std::vector<osmscout::RelationRef>::iterator relAreaIt;
                    for(relAreaIt = listRelAreaRefs.begin();
                        relAreaIt != listRelAreaRefs.end(); ++relAreaIt)
                    {
                        if(listStyleConfigs[i]->GetAreaTypeIsValid((*relAreaIt)->GetType()))
                        {
                            if(setRelAreasAllLods.insert((*relAreaIt)->GetId()).second)   {
                                listRelAreaRefsByLod[i].insert(std::make_pair((*relAreaIt)->GetId(),*relAreaIt));
                            }
                        }
                    }
                }
            }
        }   // for each LOD

        // update render data
        updateNodeRenderData(dataSet,listNodeRefsByLod);
        updateWayRenderData(dataSet,listWayRefsByLod);
        updateAreaRenderData(dataSet,listAreaRefsByLod);
        updateRelAreaRenderData(dataSet,listRelAreaRefsByLod);

    }   // for each DataSet

    this->doneUpdatingAreas();
    this->doneUpdatingRelAreas();

    // update current data extents
    m_data_exTL = m_camera.exTL;
    m_data_exTR = m_camera.exTR;
    m_data_exBL = m_camera.exBL;
    m_data_exBR = m_camera.exBR;
}

void MapRenderer::updateSceneBasedOnCamera()
{
    OSRDEBUG << "INFO: [Updating Scene Contents...]";
    updateSceneContents(m_listDataSets);

    /*
    double oldArea =
            calcTriangleArea(m_data_exTL,m_data_exTR,m_data_exBR) +
            calcTriangleArea(m_data_exTL,m_data_exBL,m_data_exBR);

    double newArea =
            calcTriangleArea(m_camera.exTL,m_camera.exTR,m_camera.exBR) +
            calcTriangleArea(m_camera.exTL,m_camera.exBL,m_camera.exBR);

    // calc overlap
    std::vector<Vec3> listVxPoly1(4);
    listVxPoly1[0] = m_data_exTL;
    listVxPoly1[1] = m_data_exTR;
    listVxPoly1[2] = m_data_exBR;
    listVxPoly1[3] = m_data_exBL;

    std::vector<Vec3> listVxPoly2(4);
    listVxPoly2[0] = m_camera.exTL;
    listVxPoly2[1] = m_camera.exTR;
    listVxPoly2[2] = m_camera.exBR;
    listVxPoly2[3] = m_camera.exBL;

    double overlapArea = 0; std::vector<Vec3> listVxOverlap;
    calcBoundsIntersection(listVxPoly1,listVxPoly2,listVxOverlap);

    if(!listVxOverlap.empty())   {
        overlapArea =
                calcTriangleArea(listVxOverlap[0],
                                     listVxOverlap[1],
                                     listVxOverlap[2]) +
                calcTriangleArea(listVxOverlap[0],
                                     listVxOverlap[3],
                                     listVxOverlap[2]);
    }

    if(oldArea < 1E-7)
    {   updateSceneContents(m_listDataSets);   return;   }

    double oldOverlap = overlapArea/oldArea;
    double newOverlap = overlapArea/newArea;

    if(oldOverlap < 0.75 || newOverlap < 0.75)
    {
        // just for debugging TODO
        showCameraViewArea(m_camera);

        // update scene contents
        OSRDEBUG << "INFO: [Updating Scene Contents...]";
        updateSceneContents(m_listDataSets);
    }
    */
}

// ========================================================================== //
// ========================================================================== //

void MapRenderer::updateNodeRenderData(DataSet *dataSet,
                                       ListNodeRefsByLod &listNodeRefs)
{
    size_t thingsAdded = 0;
    size_t thingsRemoved = 0;
    ListNodeDataByLod &listNodeData = dataSet->listNodeData;

    for(size_t i=0; i < listNodeRefs.size(); i++)
    {
        TYPE_UNORDERED_MAP<osmscout::Id,osmscout::NodeRef>::iterator itNew;
        TYPE_UNORDERED_MAP<osmscout::Id,NodeRenderData>::iterator itOld;

        // remove objects from the old view extents
        // not present in the new view extents
        for(itOld = listNodeData[i].begin();
            itOld != listNodeData[i].end();)
        {
            itNew = listNodeRefs[i].find((*itOld).first);

            if(itNew == listNodeRefs[i].end())
            {   // node dne in new view -- remove it
                TYPE_UNORDERED_MAP<osmscout::Id,NodeRenderData>::iterator itDelete = itOld;
                removeNodeFromScene((*itDelete).second); ++itOld;
                listNodeData[i].erase(itDelete);
                thingsRemoved++;
            }
            else
            {   ++itOld;   }
        }

        // add objects from the new view extents
        // not present in the old view extents
        NodeRenderData nodeRenderData;
        for(itNew = listNodeRefs[i].begin();
            itNew != listNodeRefs[i].end(); ++itNew)
        {
            itOld = listNodeData[i].find((*itNew).first);

            if(itOld == listNodeData[i].end())
            {   // node dne in old view -- add it
                if(genNodeRenderData(dataSet,(*itNew).second,
                                     dataSet->listStyleConfigs[i],
                                     nodeRenderData))
                {
                    addNodeToScene(nodeRenderData);
                    clearNodeRenderData(nodeRenderData);
                    std::pair<osmscout::Id,NodeRenderData> insPair((*itNew).first,nodeRenderData);
                    listNodeData[i].insert(insPair);
                    thingsAdded++;
                }
            }
        }
    }
}

void MapRenderer::updateWayRenderData(DataSet *dataSet,
                                      ListWayRefsByLod &listWayRefs)
{
    size_t thingsAdded = 0;
    size_t thingsRemoved = 0;
    ListWayDataByLod &listWayData = dataSet->listWayData;
    for(int i=0; i < listWayRefs.size(); i++)
    {
        TYPE_UNORDERED_MAP<osmscout::Id,osmscout::WayRef>::iterator itNew;
        TYPE_UNORDERED_MAP<osmscout::Id,WayRenderData>::iterator itOld;

        // remove objects from the old view extents
        // not present in the new view extents
        for(itOld = listWayData[i].begin();
            itOld != listWayData[i].end();)
        {
            itNew = listWayRefs[i].find((*itOld).first);

            if(itNew == listWayRefs[i].end())
            {   // way dne in new view -- remove it
                TYPE_UNORDERED_MAP<osmscout::Id,WayRenderData>::iterator itDelete = itOld;

                removeWayFromSharedNodes(dataSet,itDelete->second.wayRef);
                removeWayFromScene((*itDelete).second); ++itOld;
                listWayData[i].erase(itDelete);
                thingsRemoved++;
            }
            else
            {   ++itOld;   }
        }

        // add objects from the new view extents
        // not present in the old view extents
        WayRenderData wayRenderData;
        for(itNew = listWayRefs[i].begin();
            itNew != listWayRefs[i].end(); ++itNew)
        {
            itOld = listWayData[i].find((*itNew).first);

            if(itOld == listWayData[i].end())
            {   // way dne in old view -- add it
                if(genWayRenderData(dataSet,(*itNew).second,
                                    dataSet->listStyleConfigs[i],
                                    wayRenderData))
                {
                    addWayToScene(wayRenderData);
                    clearWayRenderData(wayRenderData);
                    std::pair<osmscout::Id,WayRenderData> insPair((*itNew).first,wayRenderData);
                    listWayData[i].insert(insPair);
                    thingsAdded++;
                }
            }
        }
    }
//    OSRDEBUG << "Ways Added: " << thingsAdded;
//    OSRDEBUG << "Ways Removed: " << thingsRemoved;
}

void MapRenderer::updateAreaRenderData(DataSet *dataSet,
                                       ListAreaRefsByLod &listAreaRefs)
{
    ListAreaDataByLod &listAreaData = dataSet->listAreaData;

    for(int i=0; i < listAreaRefs.size(); i++)
    {
        TYPE_UNORDERED_MAP<osmscout::Id,osmscout::WayRef>::iterator itNew;
        TYPE_UNORDERED_MAP<osmscout::Id,AreaRenderData>::iterator itOld;

        // remove objects from the old view extents
        // not present in the new view extents
        for(itOld = listAreaData[i].begin();
            itOld != listAreaData[i].end();)
        {
            itNew = listAreaRefs[i].find((*itOld).first);

            if(itNew == listAreaRefs[i].end())
            {   // way dne in new view -- remove it
                TYPE_UNORDERED_MAP<osmscout::Id,AreaRenderData>::iterator itDelete = itOld;
                removeAreaFromScene((*itDelete).second); ++itOld;
                listAreaData[i].erase(itDelete);
            }
            else
            {   ++itOld;   }
        }

        // add objects from the new view extents
        // not present in the old view extents
        for(itNew = listAreaRefs[i].begin();
            itNew != listAreaRefs[i].end(); ++itNew)
        {
            itOld = listAreaData[i].find((*itNew).first);

            if(itOld == listAreaData[i].end())
            {   // way dne in old view -- add it
                AreaRenderData areaRenderData;
                areaRenderData.lod = i;

                if(genAreaRenderData(dataSet,(*itNew).second,
                                     dataSet->listStyleConfigs[i],
                                     areaRenderData))
                {
                    addAreaToScene(areaRenderData);
                    clearAreaRenderData(areaRenderData);
                    std::pair<osmscout::Id,AreaRenderData> insPair((*itNew).first,areaRenderData);
                    listAreaData[i].insert(insPair);
                }
            }
        }
    }
}

void MapRenderer::updateRelWayRenderData(DataSet *dataSet,
                                         ListRelWayRefsByLod &listRelWayRefs)
{
    // todo
}

void MapRenderer::updateRelAreaRenderData(DataSet *dataSet,
                                          ListRelAreaRefsByLod &listRelAreaRefs)
{
    ListRelAreaDataByLod &listRelAreaData = dataSet->listRelAreaData;

    for(int i=0; i < listRelAreaRefs.size(); i++)
    {
        TYPE_UNORDERED_MAP<osmscout::Id,osmscout::RelationRef>::iterator itNew;
        TYPE_UNORDERED_MAP<osmscout::Id,RelAreaRenderData>::iterator itOld;

        // remove objects from the old view extents
        // not present in the new view extents
        for(itOld = listRelAreaData[i].begin();
            itOld != listRelAreaData[i].end();)
        {
            itNew = listRelAreaRefs[i].find((*itOld).first);

            if(itNew == listRelAreaRefs[i].end())
            {   // way dne in new view -- remove it
                TYPE_UNORDERED_MAP<osmscout::Id,RelAreaRenderData>::iterator itDelete = itOld;
                removeRelAreaFromScene((*itDelete).second); ++itOld;
                listRelAreaData[i].erase(itDelete);
            }
            else
            {   ++itOld;   }
        }

        // add objects from the new view extents
        // not present in the old view extents
        for(itNew = listRelAreaRefs[i].begin();
            itNew != listRelAreaRefs[i].end(); ++itNew)
        {
            itOld = listRelAreaData[i].find((*itNew).first);

            if(itOld == listRelAreaData[i].end())
            {   // way dne in old view -- add it
                RelAreaRenderData relRenderData;

                if(genRelAreaRenderData(dataSet,(*itNew).second,
                                        dataSet->listStyleConfigs[i],
                                        relRenderData))
                {
                    addRelAreaToScene(relRenderData);
                    clearRelAreaRenderData(relRenderData);
                    std::pair<osmscout::Id,RelAreaRenderData> insPair((*itNew).first,relRenderData);
                    listRelAreaData[i].insert(insPair);
                }
            }
        }
    }
}

// ========================================================================== //
// ========================================================================== //

bool MapRenderer::genNodeRenderData(DataSet *dataSet,
                                    const osmscout::NodeRef &nodeRef,
                                    const RenderStyleConfig *renderStyle,
                                    NodeRenderData &nodeRenderData)
{
    osmscout::TypeId nodeType = nodeRef->GetType();

    nodeRenderData.nodeRef = nodeRef;
    nodeRenderData.fillRenderStyle =
            renderStyle->GetNodeFillStyle(nodeType);
    nodeRenderData.symbolRenderStyle =
            renderStyle->GetNodeSymbolStyle(nodeType);

    // get node geometry
    nodeRenderData.nodePosn =
            convLLAToECEF(PointLLA(nodeRef->GetLat(),
                                   nodeRef->GetLon()));
    // node label data
    std::string nameLabel;
    for(size_t i=0; i < nodeRef->GetTagCount(); i++)  {
        if(nodeRef->GetTagKey(i) == dataSet->tagName)
        {   nameLabel = nodeRef->GetTagValue(i);   }
    }

    nodeRenderData.nameLabel = nameLabel;
    nodeRenderData.nameLabelRenderStyle =
            renderStyle->GetNodeNameLabelStyle(nodeType);
    nodeRenderData.hasName = (nodeRenderData.nameLabel.size() > 0) &&
            !(nodeRenderData.nameLabelRenderStyle == NULL);

    return true;
}

bool MapRenderer::genWayRenderData(DataSet *dataSet,
                                   const osmscout::WayRef &wayRef,
                                   const RenderStyleConfig *renderStyle,
                                   WayRenderData &wayRenderData)
{
    osmscout::TypeId wayType = wayRef->GetType();

    // set general way properties
    wayRenderData.wayRef = wayRef;
    wayRenderData.wayLayer = renderStyle->GetWayLayer(wayType);
    wayRenderData.lineRenderStyle = renderStyle->GetWayLineStyle(wayType);

    // build way geometry
    wayRenderData.listWayPoints.resize(wayRef->nodes.size());
    this->getListOfSharedWayNodes(dataSet,wayRef,
        wayRenderData.listSharedNodes);

    if(wayType == dataSet->GetTypeConfig()->GetTypeId("_tile_coastline"))
    {   // if the way is coastline data, we encode breaks
        // in the coastline with (0,0,0) points and need
        // to explicitly account for this

        for(size_t i=0; i < wayRef->nodes.size(); i++)   {
            double lat = wayRef->nodes[i].GetLat();
            double lon = wayRef->nodes[i].GetLon();
            if((lat == 0) && (lon == 0))   {
                wayRenderData.listWayPoints[i] = Vec3(0,0,0);
            }
            else   {
                wayRenderData.listWayPoints[i] =
                        convLLAToECEF(PointLLA(lat,lon,0.0));
            }
        }
        wayRenderData.isCoast = true;
    }
    else
    {   // if the way can be a street type, we need to save
        // shared nodes to get intersection data
        for(size_t i=0; i < wayRef->nodes.size(); i++)   {
            wayRenderData.listWayPoints[i] =
                    convLLAToECEF(PointLLA(wayRef->nodes[i].GetLat(),
                                           wayRef->nodes[i].GetLon(),0.0));

            std::pair<osmscout::Id,osmscout::Id>
                    nodeInWay(wayRef->nodes[i].GetId(),wayRef->GetId());

            dataSet->listSharedNodes.insert(nodeInWay);
        }
        wayRenderData.isCoast = false;
    }

    // way label data
    wayRenderData.nameLabel = wayRef->GetName();
    wayRenderData.nameLabelRenderStyle =
            renderStyle->GetWayNameLabelStyle(wayType);
    wayRenderData.hasName = (wayRenderData.nameLabel.size() > 0) &&
            !(wayRenderData.nameLabelRenderStyle == NULL);

    return true;
}

bool MapRenderer::genRelWayRenderData(DataSet *dataSet,
                                      const osmscout::RelationRef &relRef,
                                      const RenderStyleConfig *renderStyle,
                                      RelWayRenderData &relRenderData)
{}

bool MapRenderer::genAreaRenderData(DataSet *dataSet,
                                    const osmscout::WayRef &areaRef,
                                    const RenderStyleConfig *renderStyle,
                                    AreaRenderData &areaRenderData)
{
    // ensure that the area is valid before building
    // the area geometry in ecef coordinates
    double minLat = 200;
    double minLon = 200;
    double maxLat = -200;
    double maxLon = -200;
    std::vector<Vec2> listOuterPoints(areaRef->nodes.size());
    for(int i=0; i < listOuterPoints.size(); i++)
    {
        double myLat = areaRef->nodes[i].GetLat();
        double myLon = areaRef->nodes[i].GetLon();

        minLat = std::min(minLat,myLat);
        minLon = std::min(minLon,myLon);
        maxLat = std::max(maxLat,myLat);
        maxLon = std::max(maxLon,myLon);

        listOuterPoints[i].x = myLon;
        listOuterPoints[i].y = myLat;
    }

    if(!this->calcAreaIsValid(listOuterPoints))   {
        OSRDEBUG << "WARN: AreaRef " << areaRef->GetId()
                 << " is invalid";
        return false;
    }

    osmscout::TypeId areaType = areaRef->GetType();

    // check if area is a building
    double areaHeight = 0;
    areaRenderData.isBuilding = false;
    if(areaRef->GetTagCount() > 0)
    {
        for(int i=0; i < areaRef->GetTagCount(); i++)
        {
            if(areaRef->GetTagKey(i) == dataSet->tagBuilding)
            {
                std::string keyVal = areaRef->GetTagValue(i);
                if(keyVal != "no" && keyVal != "false" && keyVal != "0")
                {   areaRenderData.isBuilding = true;   }
            }

            else if(areaRef->GetTagKey(i) == dataSet->tagHeight)
            {   areaHeight = convStrToDbl(areaRef->GetTagValue(i));   }
        }
    }

    // set area data
    areaRenderData.areaRef = areaRef;
    areaRenderData.areaLayer =
            renderStyle->GetAreaLayer(areaType);
    areaRenderData.fillRenderStyle =
            renderStyle->GetAreaFillStyle(areaType);

    if(areaRenderData.isBuilding)   {
        if(areaHeight > 0)   {
            areaRenderData.buildingHeight = areaHeight;
        }
        else
        {   // calc building bbox height and use it as
            // a basis to estimate building height
            Vec3 topLeft = convLLAToECEF(PointLLA(maxLat,minLon,0));
            Vec3 btmLeft = convLLAToECEF(PointLLA(minLat,minLon,0));
            Vec3 btmRight = convLLAToECEF(PointLLA(minLat,maxLon,0));
            double buildingArea =
                    ((topLeft-btmLeft).Cross(btmRight-btmLeft)).Magnitude();
            areaRenderData.buildingHeight =
                    calcEstBuildingHeight(buildingArea);
        }
    }

    // convert area geometry to ecef
    areaRenderData.listOuterPoints.resize(listOuterPoints.size());
    for(int i=0; i < listOuterPoints.size(); i++)
    {
        areaRenderData.listOuterPoints[i] =
                convLLAToECEF(PointLLA(listOuterPoints[i].y,
                                       listOuterPoints[i].x,0.0));
    }

    // save center point
    double centerLat,centerLon;
    areaRef->GetCenter(centerLat,centerLon);
    areaRenderData.centerPoint =
            convLLAToECEF(PointLLA(centerLat,centerLon,0.0));

    // set area label
    areaRenderData.nameLabel = areaRef->GetName();
    areaRenderData.nameLabelRenderStyle =
            renderStyle->GetAreaNameLabelStyle(areaType);
    areaRenderData.hasName = (areaRenderData.nameLabel.size() > 0) &&
            !(areaRenderData.nameLabelRenderStyle == NULL);

    return true;
}

bool MapRenderer::genRelAreaRenderData(DataSet *dataSet,
                                       const osmscout::RelationRef &relRef,
                                       const RenderStyleConfig *renderStyle,
                                       RelAreaRenderData &relRenderData)
{
    // create a separate area for each ring by
    // clipping its immediate children (ie 1's are
    // children of 0, 2's are children of 1)

    // copy multipolygon ring hierarchy list
    std::vector<unsigned int> listRingHierarchy(relRef->roles.size());
    for(int i=0; i < relRef->roles.size(); i++)
    {   listRingHierarchy[i] = relRef->roles[i].ring;   }

    // add direct children for each ring in the hierarchy,
    // listDirectChildren contains ring indices
    std::vector<std::vector<unsigned int> > listDirectChildren;
    for(int i=0; i < listRingHierarchy.size()-1; i++)
    {
        std::vector<unsigned int> directChildren;
        for(int j=i+1; j < listRingHierarchy.size(); j++)
        {
            if(listRingHierarchy[j] <= listRingHierarchy[i])
            {   break;   }

            else if(listRingHierarchy[j]-1 == listRingHierarchy[i])
            {   directChildren.push_back(j);   }
        }
        listDirectChildren.push_back(directChildren);
    }
    std::vector<unsigned int> lastChild;
    listDirectChildren.push_back(lastChild);

    // create new area for each ring and its direct children
    for(int i=0; i < listRingHierarchy.size(); i++)
    {
        osmscout::TypeId areaType;

        // dont bother creating any geometry for parents with
        // typeIgnore roles, only exception is when ring == 0
        if(listRingHierarchy[i] > 0)
        {
            if(relRef->roles[i].GetType() == osmscout::typeIgnore)
            {   continue;   }

            // if ring > 0, and the role type isn't typeIgnore,
            // we need to draw the ring (its not just a clipping)
            areaType = relRef->roles[i].GetType();
        }
        else
        {   // if ring == 0, we take the area type straight
            // from the relation ref
            areaType = relRef->GetType();
        }

        std::vector<Vec2>                 listOuterPts;
        std::vector<std::vector<Vec2> >   listListInnerPts;

        double minLat = 200;   double minLon = 200;
        double maxLat = -200;  double maxLon = -200;

        // save outer ring nodes
        for(int v=0; v < relRef->roles[i].nodes.size(); v++)
        {
            double myLat = relRef->roles[i].nodes[v].GetLat();
            double myLon = relRef->roles[i].nodes[v].GetLon();

            minLat = std::min(minLat,myLat);
            minLon = std::min(minLon,myLon);
            maxLat = std::max(maxLat,myLat);
            maxLon = std::max(maxLon,myLon);

            Vec2 myPt(myLon,myLat);
            listOuterPts.push_back(myPt);
        }

        // save inner ring nodes
        for(int j=0; j < listDirectChildren[i].size(); j++)
        {
            std::vector<Vec2> listInnerPts;
            unsigned int chIdx = listDirectChildren[i][j];
            for(int v=0; v < relRef->roles[chIdx].nodes.size(); v++)
            {
                Vec2 myPt(relRef->roles[chIdx].nodes[v].GetLon(),
                                    relRef->roles[chIdx].nodes[v].GetLat());

                listInnerPts.push_back(myPt);
            }
            listListInnerPts.push_back(listInnerPts);
        }

        // we can optionally do a safety check here to ensure that
        // the polygon defined by listOuterPts and listListInnerPts
        // is simple if the triangulation method used requires it

        if(!this->calcAreaIsValid(listOuterPts,listListInnerPts))
        {
            OSRDEBUG << "WARN: AreaRef " << relRef->GetId()
                     << " is invalid";
            return false;

            // todo: there are different ways we can handle a complex
            // relation area:

            // * call 'continue': ignore this specific parent-child
            //   relationship but try to draw any others -- note that this
            //   is expensive as calcAreaIsValid is called multiple times

            // * return false: discard this entire relation area

            // (todo)
            // * partial draw: just save areas for the parent geometries
            //   where listRingHierarchy == 0 and ignore holes/clippings
        }

        // build AreaRenderData
        AreaRenderData areaData;

        // check if area is a building
        areaData.isBuilding = false;
        double areaHeight = 0;

        if(listRingHierarchy[i] == 0)
        {   // check if area is a building using tags in relation
            for(int j=0; j < relRef->GetTagCount(); j++)
            {
                if(relRef->GetTagKey(j) == dataSet->tagBuilding)
                {
                    std::string keyVal = relRef->GetTagValue(j);
                    if(keyVal != "no" && keyVal != "no" && keyVal != "0")
                    {   areaData.isBuilding = true;   }
                }
                else if(relRef->GetTagKey(j) == dataSet->tagHeight)
                {   areaHeight = convStrToDbl(relRef->GetTagValue(j));   }
            }

            if(!areaData.isBuilding)
            {   // check if area is a building using tags in role
                for(int j=0; j < relRef->roles[i].attributes.tags.size(); j++)
                {
                    osmscout::Tag myTag = relRef->roles[i].attributes.tags[j];
                    if(myTag.key == dataSet->tagBuilding)
                    {
                        std::string keyVal = myTag.value;
                        if(keyVal != "no" && keyVal != "no" && keyVal != "0")
                        {   areaData.isBuilding = true;   }
                    }
                    else if(myTag.key == dataSet->tagHeight)
                    {   areaHeight = convStrToDbl(myTag.value);   }
                }
            }

            // get area name using relation
            areaData.nameLabel = relRef->GetName();
        }
        else
        {   // check if area is a building using tags in role
            for(int j=0; j < relRef->roles[i].attributes.tags.size(); j++)
            {
                osmscout::Tag myTag = relRef->roles[i].attributes.tags[j];
                if(myTag.key == dataSet->tagBuilding)
                {
                    std::string keyVal = myTag.value;
                    if(keyVal != "no" && keyVal != "no" && keyVal != "0")
                    {   areaData.isBuilding = true;   }
                }
                else if(myTag.key == dataSet->tagHeight)
                {   areaHeight = convStrToDbl(myTag.value);   }
            }

            // get area name using attributes
            areaData.nameLabel = relRef->roles[i].attributes.name;
        }

        // set area properties/materials
        areaData.areaLayer = renderStyle->GetAreaLayer(areaType);
        areaData.fillRenderStyle = renderStyle->GetAreaFillStyle(areaType);

        if(areaData.isBuilding)   {
            if(areaHeight > 0)  {
                areaData.buildingHeight = areaHeight;
            }
            // todo else estimate height
        }

        // convert outer relation area geometry to ecef
        areaData.listOuterPoints.resize(listOuterPts.size());
        for(int j=0; j < listOuterPts.size(); j++)
        {
            areaData.listOuterPoints[j] =
                    convLLAToECEF(PointLLA(listOuterPts[j].y,
                                           listOuterPts[j].x,0.0));
        }

        // convert inner relation area geometry to ecef
        areaData.listListInnerPoints.resize(listListInnerPts.size());
        for(int j=0; j < listListInnerPts.size(); j++)
        {
            areaData.listListInnerPoints[j].resize(listListInnerPts[j].size());
            for(int k=0; k < listListInnerPts[j].size(); k++)
            {
                areaData.listListInnerPoints[j][k] =
                        convLLAToECEF(PointLLA(listListInnerPts[j][k].y,
                                               listListInnerPts[j][k].x,0.0));
            }
        }

        // center point
        double cLat,cLon;
        relRef->GetCenter(cLat,cLon);
        areaData.centerPoint = convLLAToECEF(PointLLA(cLat,cLon,0.0));

        // set area label
        areaData.nameLabelRenderStyle =
                renderStyle->GetAreaNameLabelStyle(areaType);
        areaData.hasName = (areaData.nameLabel.size() > 0) &&
                !(areaData.nameLabelRenderStyle == NULL);

        // save
        relRenderData.listAreaData.push_back(areaData);
    }

    relRenderData.relRef = relRef;

//    // debug multipolyon ring hierarchy
//    OSRDEBUG << "Relation Area ID: " << relRef->GetId();
//    OSRDEBUG << "Relation Type: " << this->getTypeName(dataSet,relRef->GetId());
//    for(size_t i = 0; i < relRef->roles.size(); i++)   {
//        OSRDEBUG << "Role " << i
//                 << ", Ring " << int(relRef->roles[i].ring)
//                 << ", Type " << relRef->roles[i].GetType();
//    }

    return true;
}

// ========================================================================== //
// ========================================================================== //

void MapRenderer::clearNodeRenderData(NodeRenderData &nodeRenderData)
{
    nodeRenderData.nameLabel.clear();
}

void MapRenderer::clearWayRenderData(WayRenderData &wayRenderData)
{
    wayRenderData.listWayPoints.clear();
    wayRenderData.listSharedNodes.clear();
    wayRenderData.nameLabel.clear();
}

void MapRenderer::clearAreaRenderData(AreaRenderData &areaRenderData)
{
    areaRenderData.listOuterPoints.clear();
    areaRenderData.listListInnerPoints.clear();
    areaRenderData.nameLabel.clear();
}

void MapRenderer::clearRelWayRenderData(RelWayRenderData &relRenderData)
{}      // RelWayRenderData isn't used yet

void MapRenderer::clearRelAreaRenderData(RelAreaRenderData &relRenderData)
{
    for(size_t i=0; i < relRenderData.listAreaData.size(); i++)   {
        clearAreaRenderData(relRenderData.listAreaData[i]);
    }
}

// ========================================================================== //
// ========================================================================== //

void MapRenderer::getListOfSharedWayNodes(DataSet *dataSet,
                                          osmscout::WayRef const &wayRef,
                                          std::vector<bool> &listSharedNodes)
{
    listSharedNodes.resize(wayRef->nodes.size());

    for(int i=0; i < wayRef->nodes.size(); i++)
    {
        size_t waysSharedByNode =
                dataSet->listSharedNodes.count(wayRef->nodes[i].GetId());

        if(waysSharedByNode > 0)
        {
            if(waysSharedByNode > 1)
            {   listSharedNodes[i] = true;   }
            else
            {   // if waysSharedByNode == 1, we have to check
                // whether or not the way that this node belongs
                // to isn't already the given way
                osmscout::Id nodeId = wayRef->nodes[i].GetId();
                osmscout::Id wayId = wayRef->GetId();

                if(dataSet->listSharedNodes.find(nodeId)->second == wayId)
                {   listSharedNodes[i] = false;   }
                else
                {   listSharedNodes[i] = true;   }
            }
        }
        else
        {   listSharedNodes[i] = false;   }
    }
}

void MapRenderer::removeWayFromSharedNodes(DataSet *dataSet,
                                           const osmscout::WayRef &wayRef)
{
    TYPE_UNORDERED_MULTIMAP<osmscout::Id,osmscout::Id>::iterator itShNode;
    std::pair<TYPE_UNORDERED_MULTIMAP<osmscout::Id,osmscout::Id>::iterator,
              TYPE_UNORDERED_MULTIMAP<osmscout::Id,osmscout::Id>::iterator> itRange;

    for(int i=0; i < wayRef->nodes.size(); i++)
    {
        itRange = dataSet->listSharedNodes.equal_range(wayRef->nodes[i].GetId());

        for(itShNode = itRange.first;
            itShNode != itRange.second;)
        {
            if(itShNode->second == wayRef->GetId())
            {   dataSet->listSharedNodes.erase(itShNode);  break;   }

            ++itShNode;
        }
    }
}

// ========================================================================== //
// ========================================================================== //




// ========================================================================== //
// ========================================================================== //

void MapRenderer::convLLAToECEF(const PointLLA &pointLLA, Vec3 &pointECEF)
{
    // conversion formula from...
    // hxxp://www.microem.ru/pages/u_blox/tech/dataconvert/GPS.G1-X-00006.pdf

    // remember to convert deg->rad
    double sinLat = sin(pointLLA.lat * K_PI/180.0f);
    double sinLon = sin(pointLLA.lon * K_PI/180.0f);
    double cosLat = cos(pointLLA.lat * K_PI/180.0f);
    double cosLon = cos(pointLLA.lon * K_PI/180.0f);

    // v = radius of curvature (meters)
    double v = ELL_SEMI_MAJOR / (sqrt(1-(ELL_ECC_EXP2*sinLat*sinLat)));
    pointECEF.x = (v + pointLLA.alt) * cosLat * cosLon;
    pointECEF.y = (v + pointLLA.alt) * cosLat * sinLon;
    pointECEF.z = ((1-ELL_ECC_EXP2)*v + pointLLA.alt)*sinLat;
}

Vec3 MapRenderer::convLLAToECEF(const PointLLA &pointLLA)
{
    Vec3 pointECEF;

    // remember to convert deg->rad
    double sinLat = sin(pointLLA.lat * K_PI/180.0f);
    double sinLon = sin(pointLLA.lon * K_PI/180.0f);
    double cosLat = cos(pointLLA.lat * K_PI/180.0f);
    double cosLon = cos(pointLLA.lon * K_PI/180.0f);

    // v = radius of curvature (meters)
    double v = ELL_SEMI_MAJOR / (sqrt(1-(ELL_ECC_EXP2*sinLat*sinLat)));
    pointECEF.x = (v + pointLLA.alt) * cosLat * cosLon;
    pointECEF.y = (v + pointLLA.alt) * cosLat * sinLon;
    pointECEF.z = ((1-ELL_ECC_EXP2)*v + pointLLA.alt)*sinLat;

    return pointECEF;
}

void MapRenderer::convECEFToLLA(const Vec3 &pointECEF, PointLLA &pointLLA)
{
    // conversion formula from...
    // hxxp://www.microem.ru/pages/u_blox/tech/dataconvert/GPS.G1-X-00006.pdf

    double p = (sqrt(pow(pointECEF.x,2) + pow(pointECEF.y,2)));
    double th = atan2(pointECEF.z*ELL_SEMI_MAJOR, p*ELL_SEMI_MINOR);
    double sinTh = sin(th);
    double cosTh = cos(th);

    // calc longitude
    pointLLA.lon = atan2(pointECEF.y, pointECEF.x);

    // calc latitude
    pointLLA.lat = atan2(pointECEF.z + ELL_ECC2_EXP2*ELL_SEMI_MINOR*sinTh*sinTh*sinTh,
                         p - ELL_ECC_EXP2*ELL_SEMI_MAJOR*cosTh*cosTh*cosTh);
    // calc altitude
    double sinLat = sin(pointLLA.lat);
    double N = ELL_SEMI_MAJOR / (sqrt(1-(ELL_ECC_EXP2*sinLat*sinLat)));
    pointLLA.alt = (p/cos(pointLLA.lat)) - N;

    // convert from rad to deg
    pointLLA.lon = pointLLA.lon * 180.0/K_PI;
    pointLLA.lat = pointLLA.lat * 180.0/K_PI;
}

PointLLA MapRenderer::convECEFToLLA(const Vec3 &pointECEF)
{
    PointLLA pointLLA;

    double p = (sqrt(pow(pointECEF.x,2) + pow(pointECEF.y,2)));
    double th = atan2(pointECEF.z*ELL_SEMI_MAJOR, p*ELL_SEMI_MINOR);
    double sinTh = sin(th);
    double cosTh = cos(th);

    // calc longitude
    pointLLA.lon = atan2(pointECEF.y, pointECEF.x);

    // calc latitude
    pointLLA.lat = atan2(pointECEF.z + ELL_ECC2_EXP2*ELL_SEMI_MINOR*sinTh*sinTh*sinTh,
                         p - ELL_ECC_EXP2*ELL_SEMI_MAJOR*cosTh*cosTh*cosTh);
    // calc altitude
    double sinLat = sin(pointLLA.lat);
    double N = ELL_SEMI_MAJOR / (sqrt(1-(ELL_ECC_EXP2*sinLat*sinLat)));
    pointLLA.alt = (p/cos(pointLLA.lat)) - N;

    // convert from rad to deg
    pointLLA.lon = pointLLA.lon * 180.0/K_PI;
    pointLLA.lat = pointLLA.lat * 180.0/K_PI;

    return pointLLA;
}

double MapRenderer::convStrToDbl(const std::string &strNum)
{
    std::istringstream iss(strNum);
    double numVal;

    if(!(iss >> numVal))
    {   numVal=0;   }

    return numVal;
}

double MapRenderer::calcTriangleArea(const Vec3 &vxA,
                                     const Vec3 &vxB,
                                     const Vec3 &vxC)
{   // using Heron's formula
    double a = vxB.DistanceTo(vxC);
    double b = vxA.DistanceTo(vxC);
    double c = vxA.DistanceTo(vxB);
    double s = (a+b+c)/2;

    double area = sqrt(s*(s-a)*(s-b)*(s-c));
    return area;
}

double MapRenderer::calcRectOverlapArea(double r1_bl_x, double r1_bl_y,
                                        double r1_tr_x, double r1_tr_y,
                                        double r2_bl_x, double r2_bl_y,
                                        double r2_tr_x, double r2_tr_y)
{
    // check if rectangles intersect
    if((r1_tr_x < r2_bl_x) || (r1_bl_x > r2_tr_x) ||
            (r1_tr_y < r2_bl_y) || (r1_bl_y > r2_tr_y))
    {   return 0;   }

    double r3_bl_x = std::max(r1_bl_x,r2_bl_x);
    double r3_bl_y = std::max(r2_bl_y,r2_bl_y);
    double r3_tr_x = std::min(r1_tr_x,r2_tr_x);
    double r3_tr_y = std::min(r2_tr_y,r2_tr_y);

    return ((r3_tr_x-r3_bl_x)*(r3_tr_y-r3_bl_y));
}

bool MapRenderer::calcLinesIntersect(double a_x1, double a_y1,
                                     double a_x2, double a_y2,
                                     double b_x1, double b_y1,
                                     double b_x2, double b_y2)
{
    double ua_numr = (b_x2-b_x1)*(a_y1-b_y1)-(b_y2-b_y1)*(a_x1-b_x1);
    double ub_numr = (a_x2-a_x1)*(a_y1-b_y1)-(a_y2-a_y1)*(a_x1-b_x1);
    double denr = (b_y2-b_y1)*(a_x2-a_x1)-(b_x2-b_x1)*(a_y2-a_y1);

    if(denr == 0.0)
    {
        // lines are coincident
        if(ua_numr == 0.0 && ub_numr == 0.0)
        {   return true;   }

        // lines are parallel
        else
        {   return false;   }
    }

    double ua = ua_numr/denr;
    double ub = ub_numr/denr;

    if(ua >= 0.0 && ua <= 1.0 && ub >= 0.0 && ub <= 1.0)
    {   return true;   }

    return false;
}

IntersectionType MapRenderer::calcLinesIntersect(double a_x1, double a_y1,
                                                 double a_x2, double a_y2,
                                                 double b_x1, double b_y1,
                                                 double b_x2, double b_y2,
                                                 double &i_x1, double &i_y1)
{
    double ua_numr = (b_x2-b_x1)*(a_y1-b_y1)-(b_y2-b_y1)*(a_x1-b_x1);
    double ub_numr = (a_x2-a_x1)*(a_y1-b_y1)-(a_y2-a_y1)*(a_x1-b_x1);
    double denr = (b_y2-b_y1)*(a_x2-a_x1)-(b_x2-b_x1)*(a_y2-a_y1);

    if(denr == 0.0)
    {
        // lines are coincident
        if(ua_numr == 0.0 && ub_numr == 0.0)
        {   return XSEC_COINCIDENT;   }

        // lines are parallel
        else
        {   return XSEC_PARALLEL;   }
    }

    double ua = ua_numr/denr;
    double ub = ub_numr/denr;

    if(ua >= 0.0 && ua <= 1.0 && ub >= 0.0 && ub <= 1.0)
    {
        i_x1 = a_x1+ua*(a_x2-a_x1);
        i_y1 = a_y1+ua*(a_y2-a_y1);
        return XSEC_TRUE;
    }

    return XSEC_FALSE;
}

bool MapRenderer::calcPointInPoly(std::vector<Vec2> const &listVx,
                                  Vec2 const &vxTest)
{
    OSRDEBUG << "### calPointInPoly:";
    for(size_t i=0; i < listVx.size(); i++)
    {   printVector(Vec3(listVx[i].x/1000,listVx[i].y/1000,0));   }

    OSRDEBUG << "### test Point";
    printVector(Vec3(vxTest.x/1000,vxTest.y/1000,0));

    // ref: hxxp://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
    int i,j;
    bool c = false;
    size_t nvert = listVx.size();

    for (i = 0, j = nvert-1; i < nvert; j = i++) {
      if ( ((listVx[i].y>vxTest.y) != (listVx[j].y>vxTest.y)) &&
            (vxTest.x < (listVx[j].x-listVx[i].x) * (vxTest.y-listVx[i].y) /
            (listVx[j].y-listVx[i].y) + listVx[i].x) )
         c = !c;
    }
    return c;
}

void MapRenderer::calcQuadraticEquationReal(double a, double b, double c,
                                            std::vector<double> &listRoots)
{
    // check discriminant
    double myDiscriminant = b*b - 4*a*c;

    if(myDiscriminant > 0)
    {
        double qSeg1 = (-1*b)/(2*a);
        double qSeg2 = sqrt(myDiscriminant)/(2*a);
        listRoots.push_back(qSeg1+qSeg2);
        listRoots.push_back(qSeg1-qSeg2);
    }
}

bool MapRenderer::calcPolyIsSimple(const std::vector<LineVec2> &listEdges,
                                   const std::vector<bool> &edgeStartsNewPoly)
{
    // NOTE: expects Vec2.x as longitude and Vec2.y as latitude
    unsigned int edgesIntersect = 0;
    for(int i=0; i < listEdges.size(); i++)  {
        edgesIntersect = 0;
        for(int j=i+1; j < listEdges.size(); j++)  {
            if(calcLinesIntersect(listEdges[i].first.x,
                                  listEdges[i].first.y,
                                  listEdges[i].second.x,
                                  listEdges[i].second.y,
                                  listEdges[j].first.x,
                                  listEdges[j].first.y,
                                  listEdges[j].second.x,
                                  listEdges[j].second.y))
            {
                edgesIntersect++;

                // we check the first edge of a sole poly against every
                // other edge and expect to see 2 intersections for
                // adjacent edges; poly is complex if there are more
                // intersections
                if(edgeStartsNewPoly[i] == true) {
                    if(edgesIntersect > 2)
                    {   return false;   }
                }

                // otherwise we check an edge that isn't the first
                // edge against every other edge excluding those that
                // have already been tested (this means one adjacent
                // edge); poly is complex if there is more than one
                // intersection
                else  {
                    if(edgesIntersect > 1)
                    {   return false;   }
                }
            }
        }
    }
    return true;
}

bool MapRenderer::calcPolyIsCCW(const std::vector<Vec2> &listPoints)
{
    // based on  hxxp://en.wikipedia.org/wiki/Curve_orientation
    // and       hxxp://local.wasp.uwa.edu.au/~pbourke/geometry/clockwise/

    // note: poly must be simple

    int ptIdx = 0;
    for(int i=1; i < listPoints.size(); i++)
    {
        // find the point with the smallest y value,
        if(listPoints[i].y < listPoints[ptIdx].y)
        {   ptIdx = i;   }

        // if y values are equal save the point with greatest x
        else if(listPoints[i].y == listPoints[ptIdx].y)
        {
            if(listPoints[i].x < listPoints[ptIdx].x)
            {   ptIdx = i;   }
        }
    }

    int prevIdx = (ptIdx == 0) ? listPoints.size()-1 : ptIdx-1;
    int nextIdx = (ptIdx == listPoints.size()-1) ? 0 : ptIdx+1;

    double signedArea = (listPoints[ptIdx].x-listPoints[prevIdx].x) *
                        (listPoints[nextIdx].y-listPoints[ptIdx].y) -
                        (listPoints[ptIdx].y-listPoints[prevIdx].y) *
                        (listPoints[nextIdx].x-listPoints[ptIdx].x);

    return (signedArea > 0.0);
}

bool MapRenderer::calcAreaIsValid(std::vector<Vec2> &listOuterPts)
{
    std::vector<std::vector<Vec2> > listListInnerPts; //empty
    return(this->calcAreaIsValid(listOuterPts,listListInnerPts));
}

bool MapRenderer::calcAreaIsValid(std::vector<Vec2> &listOuterPts,
                                  std::vector<std::vector<Vec2> > &listListInnerPts)
{
    if(listOuterPts.size() < 3)   {
        OSRDEBUG << "WARN: Area has less than three points!";
        return false;
    }

    unsigned int numEdges = listOuterPts.size();
    for(int i=0; i < listListInnerPts.size(); i++)
    {   numEdges += listListInnerPts[i].size();   }

    std::vector<bool> edgeStartsNewPoly(numEdges,false);
    std::vector<LineVec2> listEdges(numEdges);
    unsigned int cEdge = 0;

    // temporarily wrap around vertices
    // (first == last) to generate edge lists
    listOuterPts.push_back(listOuterPts[0]);
    for(int i=0; i < listListInnerPts.size(); i++)
    {   listListInnerPts[i].push_back(listListInnerPts[i][0]);   }

    // outer poly
    edgeStartsNewPoly[0] = true;
    for(int i=1;i < listOuterPts.size(); i++)
    {
        LineVec2 outerEdge;
        outerEdge.first = listOuterPts[i-1];
        outerEdge.second = listOuterPts[i];
        listEdges[cEdge] = outerEdge; cEdge++;
    }

    // inner polys
    for(int i=0; i < listListInnerPts.size(); i++)
    {
        edgeStartsNewPoly[cEdge] = true;
        for(int j=1; j < listListInnerPts[i].size(); j++)
        {
            LineVec2 innerEdge;
            innerEdge.first = listListInnerPts[i][j-1];
            innerEdge.second = listListInnerPts[i][j];
            listEdges[cEdge] = innerEdge; cEdge++;
        }
    }

    // revert vertex list modifications (not
    // really the 'nicest' way of doing this)
    listOuterPts.pop_back();
    for(int i=0; i < listListInnerPts.size(); i++)
    {   listListInnerPts[i].pop_back();   }

    if(calcPolyIsSimple(listEdges,edgeStartsNewPoly))
    {
        // expect listOuterPts to be CCW and innerPts
        // to be CW, if not then reverse point order

        if(!calcPolyIsCCW(listOuterPts))   {
            std::reverse(listOuterPts.begin(),
                         listOuterPts.end());
        }

        for(int i=0; i < listListInnerPts.size(); i++)   {
            if(calcPolyIsCCW(listListInnerPts[i]))   {
                std::reverse(listListInnerPts[i].begin(),
                             listListInnerPts[i].end());
            }
        }
    }
    else   {
        OSRDEBUG << "WARN: Area poly is complex!";
        return false;
    }

    return true;
}

void MapRenderer::calcSimplePolyCentroid(std::vector<Vec2> const &listVx,
                                         Vec2 &vxCentroid)
{
    // ref: hxxp://en.wikipedia.org/wiki/Centroid#Centroid_of_polygon
    std::vector<Vec2> lsVx = listVx;
    lsVx.push_back(lsVx[0]);    // wrap around

    vxCentroid.x = 0;
    vxCentroid.y = 0;
    double signedArea = 0;
    for(size_t i=0; i < lsVx.size()-1; i++)   {
        double area = (lsVx[i].x*lsVx[i+1].y - lsVx[i+1].x*lsVx[i].y);
        vxCentroid.x += (lsVx[i].x + lsVx[i+1].x) * area;
        vxCentroid.y += (lsVx[i].y + lsVx[i+1].y) * area;
        signedArea += area;
    }

    signedArea /= 2.0;

    double k = (1.0/(6.0*signedArea));
    vxCentroid.x *= k;
    vxCentroid.y *= k;
}





double MapRenderer::calcPolylineLength(const std::vector<Vec3> &listVx)
{
    double totalDist = 0;
    for(size_t i=0; i < listVx.size(); i++)
    {   totalDist += (listVx[i].DistanceTo(listVx[i-1]));   }

    return totalDist;
}

void MapRenderer::calcPolylineSegmentDist(const std::vector<Vec3> &listVx,
                                          std::vector<double> &listSegDistances)
{
    double totalDist = 0;
    listSegDistances.resize(listVx.size(),0);
    for(size_t i=1; i < listVx.size(); i++)   {
        totalDist += (listVx[i].DistanceTo(listVx[i-1]));
        listSegDistances[i] = totalDist;
    }
}

void MapRenderer::calcPolylineVxAtDist(std::vector<Vec3> const &listVx,
                                       double const polylineDist,
                                       Vec3 &vxAtDist)
{
    // TODO TEST THIS!
    size_t i=0; Vec3 distVec;
    double distAlongPathPrev=0;
    double distAlongPathNext=0;

    for(i=1; i < listVx.size(); i++)   {
        distVec = listVx[i]-listVx[i-1];
        distAlongPathPrev = distAlongPathNext;
        distAlongPathNext += distVec.Magnitude();

        if(distAlongPathNext >= polylineDist)
        {   break;   }
    }

    double fAlongSegment =
            (polylineDist-distAlongPathPrev)/
            (distAlongPathNext-distAlongPathPrev);

    vxAtDist = listVx[i-1]+(distVec.ScaledBy(fAlongSegment));
}

void MapRenderer::calcPolylineResample(std::vector<Vec3> const &listVx,
                                       double const distResample,
                                       std::vector<Vec3> &listVxRes)
{
    if(distResample == 0 || listVx.empty())
    {   return;   }

    double distOverlap=0;
    listVxRes.push_back(listVx[0]);

    double startBuffer=0;
    for(size_t i=1; i < listVx.size(); i++)
    {
        Vec3 vSegment = listVx[i]-listVx[i-1];

        if(startBuffer > vSegment.Magnitude())   {
            startBuffer -= vSegment.Magnitude();
            continue;
        }

        Vec3 vAddPt = vSegment.Normalized().ScaledBy(distResample);
        Vec3 vStartPt = vSegment.Normalized().ScaledBy(startBuffer);
        vStartPt = vStartPt + listVx[i-1];
        listVxRes.push_back(vStartPt);

        double availDist = vSegment.Magnitude()-startBuffer;
        size_t numDivs = availDist/distResample;
        for(size_t j=0; j < numDivs; j++)   {
            listVxRes.push_back(vStartPt+(vAddPt.ScaledBy(j+1)));
        }
        startBuffer = distResample-(availDist-(distResample*numDivs));
    }
}

double MapRenderer::calcMinPointLineDistance(const Vec3 &distalPoint,
                                             const Vec3 &endPointA,
                                             const Vec3 &endPointB)
{   // ref: http://paulbourke.net/geometry/pointline/

    Vec3 lineSegment = endPointB-endPointA;
    double uNumr = distalPoint.Dot(lineSegment)-endPointA.Dot(lineSegment);
    double uDenr = (lineSegment).Dot(lineSegment);

    if(uDenr == 0)
    {   return distalPoint.DistanceTo(endPointA);   }

    double u = uNumr/uDenr;

    // check whether or not the projection falls
    // within the line segment
    if(u < 0)
    {   return distalPoint.DistanceTo(endPointA);   }

    else if(u > 1)
    {   return distalPoint.DistanceTo(endPointB);   }

    else
    {   return distalPoint.DistanceTo(endPointA+lineSegment.ScaledBy(u));   }
}

double MapRenderer::calcMaxPointLineDistance(const Vec3 &distalPoint,
                                             const Vec3 &endPointA,
                                             const Vec3 &endPointB)
{
    double distA = distalPoint.DistanceTo(endPointA);
    double distB = distalPoint.DistanceTo(endPointB);
    return std::max(distA,distB);
}

double MapRenderer::calcMinPointPlaneDistance(const Vec3 &distalPoint,
                                              const Vec3 &planePoint,
                                              const Vec3 &planeNormal)
{
    // ref: http://paulbourke.net/geometry/pointline/

    // find the plane's coeffecients
    double a = planeNormal.x;
    double b = planeNormal.y;
    double c = planeNormal.z;
    double d = -1 * (a*planePoint.x + b*planePoint.y + c*planePoint.z);

    double distance = (a*distalPoint.x + b*distalPoint.y + c*distalPoint.z + d) /
        sqrt(a*a + b*b + c*c);

    return fabs(distance);
}

bool MapRenderer::calcPointLiesAlongRay(const Vec3 &distalPoint,
                                        const Vec3 &rayPoint,
                                        const Vec3 &rayDirn)
{
    double u = (distalPoint.x-rayPoint.x)/rayDirn.x;
    if(u >= 0)
    {
        if(distalPoint.y == (rayPoint.y + u*rayDirn.y))
        {
            if(distalPoint.z == (rayPoint.z + u*rayDirn.z))
            {
                return true;
            }
        }
    }

    return false;
}

bool MapRenderer::calcEstSkewLineProj(const Vec3 &a_p1, const Vec3 &a_p2,
                                      const Vec3 &b_p1, const Vec3 &b_p2,
                                      Vec3 &i_p)
{   // derivation/code from hxxp://paulbourke.net/geometry/lineline3d/

    Vec3 p21 = a_p2-a_p1;
    Vec3 p43 = b_p2-b_p1;

    if(fabs(p21.x) < K_EPS && fabs(p21.y) < K_EPS && fabs(p21.z) < K_EPS)
    {   return false;   }

    if(fabs(p43.x) < K_EPS && fabs(p43.y) < K_EPS && fabs(p43.z) < K_EPS)
    {   return false;   }

    Vec3 p13 = a_p1-b_p1;
    double d1343,d4321,d1321,d4343,d2121;
    d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
    d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
    d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
    d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
    d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

    double denom;
    denom = d2121 * d4343 - d4321 * d4321;
    if(fabs(denom) < K_EPS)
    {   return false;   }

    double numer = d1343 * d4321 - d1321 * d4343;
    double a_mu = numer/denom;
    i_p.x = a_p1.x + a_mu*p21.x;
    i_p.y = a_p1.y + a_mu*p21.y;
    i_p.z = a_p1.z + a_mu*p21.z;
}

bool MapRenderer::calcPointPlaneProjection(const Vec3 &planeNormal,
                                           const Vec3 &planeVx,
                                           const std::vector<Vec3> &listVx,
                                           std::vector<Vec3> &listProjVx)
{
    if(planeNormal.x == 0 &&
       planeNormal.y == 0 &&
       planeNormal.z == 0)   {
        return false;
    }

    listProjVx.clear();
    Vec3 pNormal = planeNormal.Normalized();
    for(size_t i=0; i < listVx.size(); i++)   {
        Vec3 vecDiff = listVx[i]-planeVx;
        double dist = vecDiff.Dot(pNormal);
        listProjVx.push_back(listVx[i] - pNormal.ScaledBy(dist));
    }

    return true;
}

bool MapRenderer::calcRayPlaneIntersection(const Vec3 &linePoint,
                                           const Vec3 &lineDirn,
                                           const Vec3 &planePoint,
                                           const Vec3 &planeNormal,
                                           Vec3 &intersectionPoint)
{
    // ref: http://paulbourke.net/geometry/planeline/

    // find the plane's coeffecients
    double a = planeNormal.x;
    double b = planeNormal.y;
    double c = planeNormal.z;
    double d = -1 * (a*planePoint.x + b*planePoint.y + c*planePoint.z);

    // solve for the point of intersection
    Vec3 lineOtherPoint = linePoint + lineDirn;
    double uNumr = a*linePoint.x + b*linePoint.y + c*linePoint.z + d;
    double uDenmr = a*(linePoint.x - lineOtherPoint.x) +
                    b*(linePoint.y-lineOtherPoint.y) +
                    c*(linePoint.z-lineOtherPoint.z);

    if(uDenmr == 0)           // ray is || to plane
    {   return false;   }

    if((uNumr/uDenmr) < 0)    // poi lies along opposite ray dirn
    {   return false;   }

    intersectionPoint = linePoint + lineDirn.ScaledBy(uNumr/uDenmr);
    return true;
}

bool MapRenderer::calcRayEarthIntersection(const Vec3 &linePoint,
                                           const Vec3 &lineDirn,
                                           Vec3 &nearXsecPoint)
{
    // the solution for intersection points between a ray
    // and the Earth's surface is a quadratic equation

    // first calculate the quadratic equation params:
    // a(x^2) + b(x) + c

    // a, b and c are found by substituting the parametric
    // equation of a line into the equation for a ellipsoid
    // and solving in terms of the line's parameter

    // * http://en.wikipedia.org/wiki/Ellipsoid
    // * http://gis.stackexchange.com/questions/20780/...
    //   ...point-of-intersection-for-a-ray-and-earths-surface

    std::vector<double> listRoots;

    double a = (pow(lineDirn.x,2) / ELL_SEMI_MAJOR_EXP2) +
               (pow(lineDirn.y,2) / ELL_SEMI_MAJOR_EXP2) +
               (pow(lineDirn.z,2) / ELL_SEMI_MINOR_EXP2);

    double b = (2*linePoint.x*lineDirn.x/ELL_SEMI_MAJOR_EXP2) +
               (2*linePoint.y*lineDirn.y/ELL_SEMI_MAJOR_EXP2) +
               (2*linePoint.z*lineDirn.z/ELL_SEMI_MINOR_EXP2);

    double c = (pow(linePoint.x,2) / ELL_SEMI_MAJOR_EXP2) +
               (pow(linePoint.y,2) / ELL_SEMI_MAJOR_EXP2) +
               (pow(linePoint.z,2) / ELL_SEMI_MINOR_EXP2) - 1;

    calcQuadraticEquationReal(a,b,c,listRoots);
    if(!listRoots.empty())
    {   // ensure poi lies along ray dirn
        if((listRoots[0] > 0) && (listRoots[1] > 0))
        {
            Vec3 point1;
            point1.x = linePoint.x + listRoots.at(0)*lineDirn.x;
            point1.y = linePoint.y + listRoots.at(0)*lineDirn.y;
            point1.z = linePoint.z + listRoots.at(0)*lineDirn.z;

            Vec3 point2;
            point2.x = linePoint.x + listRoots.at(1)*lineDirn.x;
            point2.y = linePoint.y + listRoots.at(1)*lineDirn.y;
            point2.z = linePoint.z + listRoots.at(1)*lineDirn.z;

            // save the point nearest to the ray's origin
            if(linePoint.DistanceTo(point1) > linePoint.DistanceTo(point2))
            {   nearXsecPoint = point2;   }
            else
            {   nearXsecPoint = point1;   }

            return true;
        }
    }
    return false;
}

bool MapRenderer::calcBoundsIntersection(std::vector<Vec3> const &listVxB1,
                                         std::vector<Vec3> const &listVxB2,
                                         std::vector<Vec3> &listVxROI,
                                         Vec3 &vxROICentroid)
{
    // to calculate the intersection of view/range bounds,
    // the input vertices are first converted into 2d by
    // projecting them onto a plane with a normal equal to
    // the camera eye vector, and then aligned to the xy-plane

    // the clipper library is then used to calculate the
    // intersecting region

    // the intersecting region is then realigned to the original
    // projection plane and reintersected with the surface of
    // the Earth

//    // DEBUG
//    OSRDEBUG << "### First Bound ";
//    for(size_t i=0; i < listVxB1.size(); i++)
//    {   printVector(listVxB1[i]);   }

//    OSRDEBUG << "### Second Bound ";
//    for(size_t i=0; i < listVxB2.size(); i++)
//    {   printVector(listVxB2[i]);   }

    // [project points]
    size_t szB1 = listVxB1.size();
    size_t szB2 = listVxB2.size();
    std::vector<Vec3> listVxAll,listVxProj;
    listVxAll.insert(listVxAll.end(),listVxB1.begin(),listVxB1.end());
    listVxAll.insert(listVxAll.end(),listVxB2.begin(),listVxB2.end());

    Vec3 pNormal = m_camera.eye;
    Vec3 pPoint = pNormal.ScaledBy(1.25);   // ensure that the plane has some
                                            // distance from the earth's surface
    if(!calcPointPlaneProjection(pNormal,pPoint,listVxAll,listVxProj))
    {   OSRDEBUG << "WARN: Could not project bounds";   return false;   }

//    OSRDEBUG << "### CameraEye:";
//    printVector(m_camera.eye);

    // [align projected points to xy]
    Vec3 zVec(0,0,1);
    Vec3 rAxis = pPoint.Cross(zVec).Normalized();
    double rAngleRads = acos(pPoint.Dot(zVec) / (pPoint.Magnitude()*zVec.Magnitude()));
    double rAngleDegs = rAngleRads*180.0/K_PI;

    for(size_t i=0; i < listVxProj.size(); i++)   {
        listVxProj[i] = listVxProj[i] - pPoint;                     // translate
        listVxProj[i] = listVxProj[i].RotatedBy(rAxis,rAngleDegs);  // rotate
    }

    // [find the intersecting region]
    std::vector<Vec2> listVxPoly1,listVxPoly2;

    for(size_t i=0; i < szB1; i++)
    {   listVxPoly1.push_back(Vec2(listVxProj[i].x,listVxProj[i].y));   }

    for(size_t i=szB1; i < listVxProj.size(); i++)
    {   listVxPoly2.push_back(Vec2(listVxProj[i].x,listVxProj[i].y));   }

    // note: ClipperLib uses 8 byte integers to represent points
    // that allow for min/max values far in excess of several
    // times the distance of the earth's radius measured in mm;
    // we multiply input value by 100 to preseve cm accuracy

    ClipperLib::Polygon poly1;
    for(size_t i=0; i < listVxPoly1.size(); i++)   {
        ClipperLib::long64 x = listVxPoly1[i].x*100.0;
        ClipperLib::long64 y = listVxPoly1[i].y*100.0;
        poly1.push_back(ClipperLib::IntPoint(x,y));
    }

    ClipperLib::Polygon poly2;
    for(size_t i=0; i < listVxPoly2.size(); i++)   {
        ClipperLib::long64 x = listVxPoly2[i].x*100.0;
        ClipperLib::long64 y = listVxPoly2[i].y*100.0;
        poly2.push_back(ClipperLib::IntPoint(x,y));
    }

    ClipperLib::Polygons listResults;
    ClipperLib::Clipper clipperObj;
    clipperObj.AddPolygon(poly1,ClipperLib::ptSubject);
    clipperObj.AddPolygon(poly2,ClipperLib::ptClip);

    if(!clipperObj.Execute(ClipperLib::ctIntersection,listResults))
    {   OSRDEBUG << "WARN: Could not calc xsec region";   return false;   }

    if(listResults.empty())   {
        return true;    // bounding regions don't intersect
    }

    // note: we expect listResults to only have one poly
    std::vector<Vec2> listVxRegionPoly(listResults[0].size());
    for(size_t i=0; i < listResults[0].size(); i++)   {
        listVxRegionPoly[i] = Vec2(double(listResults[0][i].X)/100.0,
                                   double(listResults[0][i].Y)/100.0);
    }

    // add the ROI centroid to the list
    Vec2 vxCentroid;
    calcSimplePolyCentroid(listVxRegionPoly,vxCentroid);
    listVxRegionPoly.push_back(vxCentroid);

    // [reproject xsec region onto earth's surface]
    listVxROI.clear();
    listVxROI.resize(listVxRegionPoly.size());
    for(size_t i=0; i < listVxRegionPoly.size(); i++)   {

        listVxROI[i] = Vec3(listVxRegionPoly[i].x,
                            listVxRegionPoly[i].y,0.0);

        // undo rotation and translation
        listVxROI[i] = listVxROI[i].RotatedBy(rAxis,rAngleDegs*-1.0);
        listVxROI[i] = listVxROI[i] + pPoint;

        // intersect with earth's surface
        Vec3 xsecPt;
        if(!calcRayEarthIntersection(listVxROI[i],Vec3(0,0,0)-pNormal,xsecPt))
        {   // we should never get here
            OSRDEBUG << "ERROR: Could not proj. xsec region";
            return false;
        }
        listVxROI[i] = xsecPt;
    }

    // save point in poly separately
    vxROICentroid = listVxROI[listVxROI.size()-1];
    listVxROI.pop_back();

//    OSRDEBUG << "### ROI Bound ";
//    for(size_t i=0; i < listVxROI.size(); i++)
//    {   printVector(listVxROI[i].ScaledBy(0.001));   }

//    OSRDEBUG << "### ROI Centroid: ";
//    printVector(vxROICentroid);

    return true;
}

void MapRenderer::calcECEFNorthEastDown(const PointLLA &pointLLA,
                                        Vec3 &vecNorth,
                                        Vec3 &vecEast,
                                        Vec3 &vecDown)
{
    Vec3 pOrigin = convLLAToECEF(pointLLA);

    Vec3 slightlyNorth = convLLAToECEF(PointLLA(pointLLA.lat+0.000001,
                                                pointLLA.lon,
                                                pointLLA.alt));

    Vec3 slightlyEast = convLLAToECEF(PointLLA(pointLLA.lat,
                                               pointLLA.lon+0.000001,
                                               pointLLA.alt));

    Vec3 slightlyDown = convLLAToECEF(PointLLA(pointLLA.lat,
                                               pointLLA.lon,
                                               pointLLA.alt-50));

    vecNorth = (slightlyNorth-pOrigin).Normalized();
    vecEast = (slightlyEast-pOrigin).Normalized();
    vecDown = (slightlyDown-pOrigin).Normalized();
}

void MapRenderer::calcGeographicDistance(const PointLLA &pointStart,
                                         double bearingDegrees,
                                         double distanceMeters,
                                         PointLLA &pointDest)
{
    // ref: http://www.movable-type.co.uk/scripts/latlong.html

    if(distanceMeters > CIR_AV/4)   {
        // prevent 'wrapping around' by clipping the max distance
        // to a quarter of the earth's av. circumference
        distanceMeters = CIR_AV/4;
    }

    double bearingRad = bearingDegrees * K_PI/180.0;
    double angularDist = distanceMeters / ELL_SEMI_MAJOR;
    double lat1 = pointStart.lat * K_PI/180.0;
    double lon1 = pointStart.lon * K_PI/180.0;

    pointDest.lat = asin(sin(lat1)*cos(angularDist) +
                         cos(lat1)*sin(angularDist)*cos(bearingRad));

    pointDest.lon = lon1 + atan2(sin(bearingRad)*sin(angularDist)*cos(lat1),
                          cos(angularDist)-sin(lat1)*sin(pointDest.lat));

    // convert back to degrees
    pointDest.lat *= (180.0/K_PI);
    pointDest.lon *= (180.0/K_PI);

    if(pointDest.lon > 180.0)   {
        pointDest.lon -= 360.0;
    }
    else if(pointDest.lon < -180.0)  {
        pointDest.lon += 360.0;
    }
}

void MapRenderer::calcDistBoundingBox(const PointLLA &ptCenter,
                                      double distMeters,
                                      Vec3 &exTL,Vec3 &exTR,
                                      Vec3 &exBR,Vec3 &exBL)
{
    std::vector<PointLLA> listPointLLA(4);
    calcGeographicDistance(ptCenter,315,distMeters,listPointLLA[0]);     // north west
    calcGeographicDistance(ptCenter,45,distMeters,listPointLLA[1]);      // north east
    calcGeographicDistance(ptCenter,135,distMeters,listPointLLA[2]);     // south east
    calcGeographicDistance(ptCenter,225,distMeters,listPointLLA[3]);     // south west

    exTL = convLLAToECEF(listPointLLA[0]);
    exTR = convLLAToECEF(listPointLLA[1]);
    exBR = convLLAToECEF(listPointLLA[2]);
    exBL = convLLAToECEF(listPointLLA[3]);
}

void MapRenderer::calcEnclosingGeoBounds(std::vector<Vec3> const &listVxPoly,
                                         std::vector<GeoBounds> &listBounds)
{
    listBounds.clear();
    if(listVxPoly.size() < 3)
    {   return;   }

    std::vector<PointLLA> listPLLA(listVxPoly.size());
    for(size_t i=0; i < listPLLA.size(); i++)
    {   listPLLA[i] = convECEFToLLA(listVxPoly[i]);   }

    // [longitude range]
    // TODO: desc using 'walking along the polygon' method

    // convert longitudes to cartesian
    std::vector<double> listLonRads(listPLLA.size());
    std::vector<Vec2> listVxLon(listPLLA.size());
    for(size_t i=0; i < listPLLA.size(); i++)   {
        double angleRads = listPLLA[i].lon*K_PI/180.0;
        listLonRads[i] = angleRads;
        listVxLon[i].x = cos(angleRads);
        listVxLon[i].y = sin(angleRads);
    }

    // walk along the polygon keeping track of max/min
    // angles; direction is relative to (x,y) = (0,0)
    bool is360 = false;
    Vec3 vecCenter(0,0,0);
    listVxLon.push_back(listVxLon[0]);      // wrap around
    listLonRads.push_back(listLonRads[0]);  // wrap around

    double diffAngle = 0;
    double angleRads = 0;
    double travelRadsCW = 0;
    double travelRadsCCW = 0;

    for(size_t i=1; i < listVxLon.size(); i++)
    {
        diffAngle = listLonRads[i]-listLonRads[i-1];

        if(diffAngle == 0)              {
            // adjacent points w/ same lon
            continue;
        }
        else if(diffAngle > K_PI)       {
            diffAngle -= (2*K_PI);
        }
        else if(diffAngle < K_PI*-1)    {
            diffAngle += (2*K_PI);
        }
        diffAngle = fabs(diffAngle);


        Vec3 vecLon(listVxLon[i-1].x,listVxLon[i-1].y,0);
        Vec3 vecDirn(listVxLon[i].x-vecLon.x,
                     listVxLon[i].y-vecLon.y,0);

        if(vecDirn.x == 0 && vecDirn.y == 0)   {
            // adjacent points w/ same longitude
            continue;
        }

        // using the right-hand rule, upVec determines
        // whether we're traversing the polygon edge CW
        // or CCW wrt to the center
        Vec3 vecUp = (vecLon-vecCenter).Cross(vecDirn);
        if(vecUp.z > 0)         {       // CCW
            angleRads += diffAngle;
            travelRadsCCW = std::max(travelRadsCCW,angleRads);
        }
        else if(vecUp.z < 0)    {       // CW
            angleRads -= diffAngle;
            travelRadsCW = std::min(travelRadsCW,angleRads);
        }
        else   {    // passes through the center
            is360 = true;
            break;
        }
    }

    double degCCW = travelRadsCCW*180.0/K_PI;
    double degCW  = travelRadsCW *180.0/K_PI;

    // TODO (comparison to 360 needs to be inexact)
    if(is360 || fabs(degCCW)>=360.0 || fabs(degCW)>=360.0)   {
        GeoBounds b; b.minLon = -180.0; b.maxLon = 180.0;
        listBounds.push_back(b);
        is360 = true;
    }
    else   {
        double startLon = listPLLA[0].lon;

        if(startLon+degCCW > 180.0)   {
            double maxLon = startLon+degCCW; calcValidAngle(maxLon);
            GeoBounds a; a.minLon = -180.0; a.maxLon = maxLon;
            GeoBounds b; b.minLon = startLon+degCW; b.maxLon = 180.0;
            listBounds.push_back(a);
            listBounds.push_back(b);
        }
        else if(startLon+degCW < -180.0)   {
            double minLon = startLon+degCW; calcValidAngle(minLon);
            GeoBounds a; a.minLon = minLon; a.maxLon = 180.0;
            GeoBounds b; b.minLon = -180.0; b.maxLon = startLon+degCCW;
            listBounds.push_back(a);
            listBounds.push_back(b);
        }
        else   {
            GeoBounds b;
            b.minLon = startLon+degCW;
            b.maxLon = startLon+degCCW;
            listBounds.push_back(b);
        }
    }

    // [latitude range]

    // we need to address the possibility that the max
    // and min latitude lie on the surface defined by
    // the vertices comprising the spherical polygon

    // formally finding the latitude range for an arbitrary
    // spherical polygon is complex, so we only look at the
    // case where the polygon spans the north or south pole
    // by doing a projected point in polygon test

    // we determine which pole to check by seeing whether
    // the camera is north or south of the equator

    if(is360)
    {
        // check if the camera is above or below the 'equator'
        double critLat = (m_camera.eye.Dot(Vec3(0,0,1)) >= 0) ? 90 : -90;
        listPLLA.push_back(PointLLA(critLat,0,0));
        OSRDEBUG << "### is360 and critLat @ " << critLat;
    }

    // calc min/max for latitude
    double minLat = 185; double maxLat = -185;
    for(size_t i=0; i < listPLLA.size(); i++)   {
        minLat = std::min(minLat,listPLLA[i].lat);
        maxLat = std::max(maxLat,listPLLA[i].lat);
    }

    // save lat
    for(size_t i=0; i < listBounds.size(); i++)   {
        listBounds[i].minLat = minLat;
        listBounds[i].maxLat = maxLat;

//        OSRDEBUG << "### Range: " << i;
//        OSRDEBUG << "### minLon: " << listBounds[i].minLon
//                 << ", maxLon: " << listBounds[i].maxLon;
//        OSRDEBUG << "### minLat: " << listBounds[i].minLat
//                 << ", maxLat: " << listBounds[i].maxLat;
    }
}

/*
void MapRenderer::calcEnclosingGeoBounds(std::vector<Vec3> const &listVxPoly,
                                         std::vector<GeoBounds> &listBounds,
                                         Vec3 const &vxCentroid)
{
    // vxCentroid is only used to improve the
    // latitude bounds calculation
    calcEnclosingGeoBounds(listVxPoly,listBounds);

    std::vector<PointLLA> listPLLA(listVxPoly.size());
    for(size_t i=0; i < listPLLA.size(); i++)
    {   listPLLA[i] = convECEFToLLA(listVxPoly[i]);   }
    listPLLA.push_back(convECEFToLLA(vxCentroid));

    // recalculate latitude range with the
    // centroid for a more accurate bounds
}
*/

bool MapRenderer::calcCamViewExtents(Camera &cam)
{
    // calculate the four edge vectors of the view frustum
    double fovy_rad_bi = (cam.fovY*K_PI/180.0)/2.0;
    double dAlongViewPt = cos(fovy_rad_bi);
    double dAlongUp = sin(fovy_rad_bi);
    double dAlongRight = dAlongUp*cam.aspectRatio;

    Vec3 camRight = (cam.viewPt-cam.eye).Cross(cam.up);
    Vec3 vAlongUp = cam.up.Normalized().ScaledBy(dAlongUp);
    Vec3 vAlongRight = camRight.Normalized().ScaledBy(dAlongRight);
    Vec3 vAlongViewPt = (cam.viewPt-cam.eye).Normalized().ScaledBy(dAlongViewPt);

    Vec3 viewTL = vAlongViewPt + vAlongUp - vAlongRight;
    Vec3 viewTR = vAlongViewPt + vAlongUp + vAlongRight;
    Vec3 viewBL = vAlongViewPt - vAlongUp - vAlongRight;
    Vec3 viewBR = vAlongViewPt - vAlongUp + vAlongRight;

    std::vector<Vec3> listProjVectors(4);
    listProjVectors[0] = viewTL;
    listProjVectors[1] = viewTR;
    listProjVectors[2] = viewBR;
    listProjVectors[3] = viewBL;

    // determine the camera parameters based on which
    // frustum edge vectors intersect with the Earth
    std::vector<bool> listIntersectsEarth(4);
    std::vector<Vec3> listIntersectionPoints(4);
    size_t numXSec = 0;     // number of vectors that xsec
                            // the Earth's surface

//    Vec3 planeOffset =

    for(size_t i=0; i < listProjVectors.size(); i++)
    {
        listIntersectsEarth[i] =
                calcRayEarthIntersection(cam.eye,
                                         listProjVectors[i],
                                         listIntersectionPoints[i]);

        if(!listIntersectsEarth[i])
        {
            // if any frustum vectors do not intersect Earth's surface,
            // intersect the vectors with a plane through the Earth
            // that is normal to the camera's view direction

            // (the corresponding POI represents the horizon at some
            // arbitrary height -- we ignore altitude data anyway)

            bool intersectsPlane =
                calcRayPlaneIntersection(cam.eye,listProjVectors[i],
                                         Vec3(0,0,0),(cam.viewPt-cam.eye),
                                         listIntersectionPoints[i]);

            // if the any of the camera vectors do not intersect the
            // center plane, assume the camera is invalid
            if(!intersectsPlane)
            {   return false;   }
        }
        else
        {   numXSec++;   }
    }

    // [TODO calc near and far planes]

    // save view extents
    cam.exTL = listIntersectionPoints[0];
    cam.exTR = listIntersectionPoints[1];
    cam.exBR = listIntersectionPoints[2];
    cam.exBL = listIntersectionPoints[3];

//    this->printCamera(m_camera);
    return true;
}

void MapRenderer::calcCamViewDistances(double &minViewDist,
                                       double &maxViewDist)
{
    std::vector<Vec3> listVx(5);
    listVx[0] = m_camera.exTL;
    listVx[1] = m_camera.exTR;
    listVx[2] = m_camera.exBR;
    listVx[3] = m_camera.exBL;
    calcRayEarthIntersection(m_camera.eye,
        m_camera.viewPt-m_camera.eye,listVx[4]);


    // get the min/max distances
    double minViewDist2,maxViewDist2;
    maxViewDist2 = -1;
    minViewDist2 = m_camera.eye.Distance2To(listVx[0]);
    for(size_t i=0; i < listVx.size(); i++)   {
        double cDist = m_camera.eye.Distance2To(listVx[i]);
        minViewDist2 = std::min(cDist,minViewDist2);
        maxViewDist2 = std::max(cDist,maxViewDist2);
    }

    minViewDist = sqrt(minViewDist2);
    maxViewDist = sqrt(maxViewDist2);
}
/*
void MapRenderer::calcCamViewDistances(double &minViewDist,
                                       double &maxViewDist)
{
    // to calculate the camera's minimum and maximum view distance,
    // we project the latitude/longitude bounding box defined by the
    // camera's view frustum onto the surface of the earth to define
    // our view surface

    // we then find the minimum and maximum distances between the
    // view surface and the camera eye

    // analytically this would involve constraining the ellipsoid
    // defining the earth's surface with planes defined by the
    // bounding box and solving for the min/max distances to a distal
    // point through ie. vector calc and lagrangian multipliers

    // however we try to brute force this numerically to obtain an
    // approximate solution; we create a mesh representing the view
    // surface and calculate the min/max distances between each vertex
    // in the mesh and the camera eye

    // get extents in lon/lat
    Vec3 vxIn = (m_camera.exTL + m_camera.exTR +
                 m_camera.exBR).ScaledBy(1.0/3.0);

    std::vector<Vec3> listVxExtents(4);
    listVxExtents[0] = m_camera.exTL;
    listVxExtents[1] = m_camera.exTR;
    listVxExtents[2] = m_camera.exBR;
    listVxExtents[3] = m_camera.exBL;

    GeoBounds bounds;
    calcEnclosingGeoBounds(listVxExtents,vxIn,bounds);

    // create the the view surface
    size_t lonSegments = 4;
    size_t latSegments = 4;
    std::vector<Vec3> listVx((lonSegments+1)*(latSegments+1));
    double lonStep = (bounds.maxLon-bounds.minLon)/lonSegments;
    double latStep = (bounds.maxLat-bounds.minLat)/latSegments;

    size_t vIdx = 0;
    for(size_t i=0; i <= latSegments; i++)   {
        for(size_t j=0; j <= lonSegments; j++)   {
            listVx[vIdx] = convLLAToECEF(PointLLA((i*latStep)+bounds.minLat,
                                                  (j*lonStep)+bounds.minLon));
            vIdx++;
        }
    }

    // get the min/max distances
    double minViewDist2,maxViewDist2;
    minViewDist2 = m_camera.eye.Distance2To(listVx[0]);
    maxViewDist2 = minViewDist2;
    for(size_t i=1; i < listVx.size(); i++)   {
        double cDist = m_camera.eye.Distance2To(listVx[i]);
        minViewDist2 = std::min(cDist,minViewDist2);
        maxViewDist2 = std::max(cDist,maxViewDist2);
    }

    minViewDist = sqrt(minViewDist2);
    maxViewDist = sqrt(maxViewDist2);
}
*/
double MapRenderer::calcEstBuildingHeight(double baseArea)
{
    // use the baseArea as a random seed
    srand(int(baseArea));

    // we estimate buildings to have a height of 3-5m per level
    // and randomly select a value within this height range
    int levelHeight = rand()%3 + 3;

    // if the baseArea of the building is below 3500 sqft
    // (~350m^2), it's likely a residential building
    if(baseArea < 350)
    {   // we assume residential buildings have two levels
        return double(levelHeight*2);
    }
    else
    {   // randomly generate number of levels building has,
        // 3-8 levels are currently used as the range
        int numLevels = rand()%6 + 3;
        return double(numLevels*levelHeight);
    }
}

void MapRenderer::calcValidAngle(double &angle)
{
    // TODO: make valid for any angle
    //       not just 0-360

    if(angle > 180.0)   {
        angle -= 360.0;
    }
    else if(angle < -180.0)   {
        angle += 360.0;
    }
}

ColorRGBA MapRenderer::calcRainbowGradient(double cVal)
{
    // clamp cVal between 0 and 1
    if(cVal < 0)   {
        cVal = 0.0;
    }
    if(cVal > 1)   {
        cVal = 1.0;
    }

    unsigned char R,G,B;
    size_t maxBars = 6;     // number of color bars

    double m = maxBars * cVal;
    size_t n = size_t(m);

    double fraction = m-n;
    unsigned char t = int(fraction*255);

    switch(n)   {
        case 0:   {
            R = 255;
            G = t;
            B = 0;
            break;
        }
        case 1:   {
            R = 255 - t;
            G = 255;
            B = 0;
            break;
        }
        case 2:   {
            R = 0;
            G = 255;
            B = t;
            break;
        }
        case 3:   {
            R = 0;
            G = 255 - t;
            B = 255;
            break;
        }
        case 4:   {
            R = t;
            G = 0;
            B = 255;
            break;
        }
        case 5:   {
            R = 255;
            G = 0;
            B = 255 - t;
            break;
        }
    }

    ColorRGBA myColor;
    myColor.R = R/255.0;
    myColor.G = G/255.0;
    myColor.B = B/255.0;
    myColor.A = 1.0;

    return myColor;
}

void MapRenderer::buildPolylineAsTriStrip(std::vector<Vec3> const &polyLine,
                                          double lineWidth,
                                          OutlineType outlineType,
                                          std::vector<Vec3> &vertexArray)
{
    unsigned int numPts = polyLine.size();
    unsigned int numOffsets = (numPts*2)-2;                 // two for every point that isn't and endpoint
    std::vector<Vec3> listLeftOffsetPts(numOffsets);
    std::vector<Vec3> listRightOffsetPts(numOffsets);
    std::vector<Vec3> listEdgeNorms(numPts-1);
    std::vector<Vec3> listEdgeDirns(numPts-1);

    Vec3 vecPlaneNormal;                                    // vector originating at the center of the earth
                                                            // (0,0,0) to a vertex on the way

    Vec3 vecAlongSegment;                                   // vector along a given segment on the way

    Vec3 vecNormToSegment;                                  // vector normal to both vecPlaneNormal and
                                                            // vecAlongSegment (used to create offset)

    Vec3 vecLeftOffset,vecRightOffset;                      // offsets from way center line vertices

    // for each segment, offset the start and end vertices
    for(int i=1; i < numPts; i++)   {
        vecPlaneNormal = polyLine[i];
        vecAlongSegment = polyLine[i]-polyLine[i-1];
        vecNormToSegment = vecAlongSegment.Cross(vecPlaneNormal).Normalized();

        listEdgeDirns[i-1] = vecAlongSegment;
        listEdgeNorms[i-1] = vecNormToSegment;
    }

    unsigned int k=0;
    switch(outlineType)
    {
        case OL_CENTER:
        {
            for(int i=1; i < numPts; i++)   {
                vecRightOffset = listEdgeNorms[i-1].ScaledBy(lineWidth/2);
                vecLeftOffset = vecRightOffset.ScaledBy(-1);

                listRightOffsetPts[k] = polyLine[i-1]+vecRightOffset;
                listLeftOffsetPts[k] = polyLine[i-1]+vecLeftOffset;
                listRightOffsetPts[k+1] = polyLine[i]+vecRightOffset;
                listLeftOffsetPts[k+1] = polyLine[i]+vecLeftOffset;
                k+=2;
            }
            break;
        }
        case OL_RIGHT:
        {
            for(int i=1; i < numPts; i++)   {
                vecRightOffset = listEdgeNorms[i-1].ScaledBy(lineWidth);

                listRightOffsetPts[k] = polyLine[i-1]+vecRightOffset;
                listLeftOffsetPts[k] = polyLine[i-1];
                listRightOffsetPts[k+1] = polyLine[i]+vecRightOffset;
                listLeftOffsetPts[k+1] = polyLine[i];
                k+=2;
            }
            break;
        }
        case OL_LEFT:
        {
            for(int i=1; i < numPts; i++)   {
                vecLeftOffset = listEdgeNorms[i-1].ScaledBy(lineWidth*-1);

                listRightOffsetPts[k] = polyLine[i-1];
                listLeftOffsetPts[k] = polyLine[i-1]+vecLeftOffset;
                listRightOffsetPts[k+1] = polyLine[i];
                listLeftOffsetPts[k+1] = polyLine[i]+vecLeftOffset;
                k+=2;
            }
            break;
        }
    }

    // todo try using clipper library to generate better offsets

    // build primitive set (triangle strips) for way
    k=0;
    vertexArray.resize(listLeftOffsetPts.size()*2);
    for(int i=0; i < listLeftOffsetPts.size(); i++)   {
        vertexArray[k] = listLeftOffsetPts[i];  k++;
        vertexArray[k] = listRightOffsetPts[i]; k++;
    }
}

void MapRenderer::buildContourSideWalls(const std::vector<Vec3> &listContourVx,
                                        const Vec3 &offsetHeight,
                                        std::vector<Vec3> &listSideTriVx,
                                        std::vector<Vec3> &listSideTriNx)
{
    if(listContourVx.size() < 3)   {
        return;
    }

    std::vector<Vec3> const &listBtmVx = listContourVx;
    std::vector<Vec3> listTopVx(listBtmVx.size());

    for(size_t i=0; i < listTopVx.size(); i++)
    {   listTopVx[i] = listBtmVx[i] + offsetHeight;   }

    // we append onto listSideTriVx and listSideTriNx
    // without clearing/modifying it so that multiple
    // geometries can be built up

    size_t v=0;
    Vec3 alongLeft,alongUp,triNx;
    for(v=0; v < listBtmVx.size()-1; v++)
    {
        // triangle 1
        listSideTriVx.push_back(listBtmVx[v]);
        listSideTriVx.push_back(listBtmVx[v+1]);
        listSideTriVx.push_back(listTopVx[v+1]);

        // triangle 2
        listSideTriVx.push_back(listBtmVx[v]);
        listSideTriVx.push_back(listTopVx[v+1]);
        listSideTriVx.push_back(listTopVx[v]);

        // normal
        alongUp = (listTopVx[v]-listBtmVx[v]);
        alongLeft = (listBtmVx[v]-listBtmVx[v+1]);

        triNx = alongUp.Cross(alongLeft).Normalized();
        listSideTriNx.insert(listSideTriNx.end(),6,triNx);
    }

    // v is now pointing to the last vertex
    // in the contour so add the last face

    // triangle 1
    listSideTriVx.push_back(listBtmVx[v]);
    listSideTriVx.push_back(listBtmVx[0]);
    listSideTriVx.push_back(listTopVx[0]);

    // triangle 2
    listSideTriVx.push_back(listBtmVx[v]);
    listSideTriVx.push_back(listTopVx[0]);
    listSideTriVx.push_back(listTopVx[v]);

    // normal
    alongUp = (listTopVx[0]-listBtmVx[0]);
    alongLeft = (listBtmVx[v]-listBtmVx[0]);
    triNx = alongUp.Cross(alongLeft).Normalized();
    listSideTriNx.insert(listSideTriNx.end(),6,triNx);
}

bool MapRenderer::buildEarthSurfaceGeometry(unsigned int latSegments,
                                            unsigned int lonSegments,
                                            std::vector<Vec3> &myVertices,
                                            std::vector<Vec3> &myNormals,
                                            std::vector<Vec2> &myTexCoords,
                                            std::vector<unsigned int> &myIndices)
{   // TODO (broken?)
    Vec3 pointECEF;
    double latStepSize = 180.0f / double(latSegments);
    double lonStepSize = 360.0f / double(lonSegments);

    if(latSegments < 4 || lonSegments < 4)
    {   // we want at least 4 lat segments and 4 lon segments
        OSRDEBUG << "ERROR: Insufficient lat/lon segments for Earth geometry";
        return false;
    }

    for(int i=0; i <= latSegments; i++)
    {
        for(int j=0; j <= lonSegments; j++)
        {
            double myLat = 90.0 - (i*latStepSize);
            double myLon = -180.0 + (j*lonStepSize);

            convLLAToECEF(PointLLA(myLat,myLon),pointECEF);
            myVertices.push_back(pointECEF);
            myNormals.push_back(myVertices.back());
            myTexCoords.push_back(Vec2(((myLon+180.0)/360.0),
                                          (myLat+90.0)/180.0));
        }
    }

    for(int i=0; i < latSegments; i++)
    {
        int pOffset = (lonSegments+1)*i;

        if(i != latSegments-1)
        {
            for(int j=pOffset; j < pOffset+lonSegments+1; j++)
            {
                myIndices.push_back(j);
                myIndices.push_back(j+lonSegments);
                myIndices.push_back(j+lonSegments+1);

                if(i > 0)
                {
                    myIndices.push_back(j);
                    myIndices.push_back(j+lonSegments+1);
                    myIndices.push_back(j+1);
                }
            }
        }
        else
        {
            for(int j=pOffset; j < pOffset+lonSegments+1; j++)
            {
                myIndices.push_back(j+lonSegments+1);
                myIndices.push_back(j+1);
                myIndices.push_back(j);
            }
        }
    }

    return true;
}

bool MapRenderer::buildEarthSurfaceGeometry(double minLon, double minLat,
                                            double maxLon, double maxLat,
                                            size_t lonSegments,
                                            size_t latSegments,
                                            std::vector<Vec3> &vertexArray,
                                            std::vector<Vec2> &texCoords,
                                            std::vector<size_t> &triIdx)
{
    if((!(minLon < maxLon)) || (!(minLat < maxLat)))   {
        OSRDEBUG << "ERROR: EarthSurfaceGeometry: Invalid bounds";
        return false;
    }

    if(latSegments < 4 || lonSegments < 4)   {
        OSRDEBUG << "ERROR: EarthSurfaceGeometry: Insufficient segments";
        return false;
    }

    double lonStep = (maxLon-minLon)/lonSegments;
    double latStep = (maxLat-minLat)/latSegments;

    vertexArray.clear();
    texCoords.clear();
    triIdx.clear();

    // build vertex attributes
    for(size_t i=0; i <= latSegments; i++)   {
        for(size_t j=0; j <= lonSegments; j++)   {
            // surface vertex
            vertexArray.push_back(convLLAToECEF(PointLLA((i*latStep)+minLat,
                                                         (j*lonStep)+minLon)));
            // surface tex coord
            texCoords.push_back(Vec2((j*lonStep)/(maxLon-minLon),
                                     (i*latStep)/(maxLat-minLat)));
        }
    }

    // stitch faces together
    size_t vIdx=0;
    for(size_t i=0; i < latSegments; i++)   {
        for(size_t j=0; j < lonSegments; j++)   {
            triIdx.push_back(vIdx);
            triIdx.push_back(vIdx+lonSegments+1);
            triIdx.push_back(vIdx+lonSegments+2);

            triIdx.push_back(vIdx);
            triIdx.push_back(vIdx+lonSegments+2);
            triIdx.push_back(vIdx+1);

            vIdx++;
        }
        vIdx++;
    }

    return true;
}

bool MapRenderer::buildCoastlinePointCloud(std::string const &filePath,
                                           std::vector<Vec3> &listVx)
{
    if(filePath.empty())
    {   return false;   }

    listVx.clear();

    // open input filePath
    CTMcontext       ctmContext;
    CTMuint          ctmVxCount;
    CTMfloat const * ctmListVx;

    ctmContext = ctmNewContext(CTM_IMPORT);
    ctmLoad(ctmContext,filePath.c_str());
    if(ctmGetError(ctmContext) == CTM_NONE)
    {
        ctmVxCount = ctmGetInteger(ctmContext,CTM_VERTEX_COUNT);
        ctmListVx  = ctmGetFloatArray(ctmContext,CTM_VERTICES);

        // save vertices
        size_t k=0;
        for(size_t i=0; i < ctmVxCount; i++)   {
            Vec3 mVx;
            mVx.x = ctmListVx[k]; k++;
            mVx.y = ctmListVx[k]; k++;
            mVx.z = ctmListVx[k]; k++;

            // a (0,0,0) vector denotes a special case
            // that we don't make use of for a point cloud
            // so it is ignored
            if(!((mVx.x == 0) && (mVx.y == 0) && (mVx.z == 0)))
            {   listVx.push_back(mVx);   }
        }
    }
    else   {
        OSRDEBUG << "ERROR: Could not read coastline0 CTM file";
        return false;
    }
    OSRDEBUG << "INFO: Read in coastline0 CTM file";
    ctmFreeContext(ctmContext);

    return true;
}

bool MapRenderer::buildCoastlineLines(std::string const &filePath,
                                      std::vector<Vec3> &listVx,
                                      std::vector<unsigned int> &listIx)
{
    if(filePath.empty())
    {   return false;   }

    listVx.clear();
    listIx.clear();

    // open input filePath
    CTMcontext       ctmContext;
    CTMuint          ctmVxCount;
    CTMfloat const * ctmListVx;

    ctmContext = ctmNewContext(CTM_IMPORT);
    ctmLoad(ctmContext,filePath.c_str());
    if(ctmGetError(ctmContext) == CTM_NONE)
    {
        ctmVxCount = ctmGetInteger(ctmContext,CTM_VERTEX_COUNT);
        ctmListVx  = ctmGetFloatArray(ctmContext,CTM_VERTICES);

        // save vertices
        size_t k=0;
        for(size_t i=0; i < ctmVxCount; i++)   {
            Vec3 mVx;
            mVx.x = ctmListVx[k]; k++;
            mVx.y = ctmListVx[k]; k++;
            mVx.z = ctmListVx[k]; k++;
            listVx.push_back(mVx);
        }
    }
    else   {
        OSRDEBUG << "ERROR: Could not read coastline0 CTM file: " << filePath;
        return false;
    }
    OSRDEBUG << "INFO: Read in coastline0 CTM file";
    ctmFreeContext(ctmContext);

    // build a list of indices for the GL_LINES primitive;
    // (0,0,0) vertices denote the start of a new polygon
    for(size_t i=0; i < listVx.size(); i++)
    {
        if((listVx[i].x == 0) &&
           (listVx[i].y == 0) &&
           (listVx[i].z == 0))
        {
            if(listIx.size() > 0)
            {   listIx.pop_back();   }

            i++;
            listIx.push_back(i);
            continue;
        }
        listIx.push_back(i);
        listIx.push_back(i);
    }
    return true;
}

bool MapRenderer::buildAdmin0Lines(const std::string &filePath,
                                   std::vector<Vec3> &listVx,
                                   std::vector<size_t> &listIx)
{
    if(filePath.empty())
    {   return false;   }

    listVx.clear();
    listIx.clear();

    // read in mesh
    CTMcontext       ctmContext;
    CTMuint          ctmVxCount;
    CTMfloat const * ctmListVx;

    ctmContext = ctmNewContext(CTM_IMPORT);
    ctmLoad(ctmContext,filePath.c_str());
    if(ctmGetError(ctmContext) == CTM_NONE)
    {
        ctmVxCount = ctmGetInteger(ctmContext,CTM_VERTEX_COUNT);
        ctmListVx  = ctmGetFloatArray(ctmContext,CTM_VERTICES);

        // save vertices
        size_t k=0;
        for(size_t i=0; i < ctmVxCount; i++)   {
            Vec3 mVx;
            mVx.x = ctmListVx[k]; k++;
            mVx.y = ctmListVx[k]; k++;
            mVx.z = ctmListVx[k]; k++;
            listVx.push_back(mVx);
        }
    }
    else   {
        OSRDEBUG << "ERROR: Could not read admin0 CTM file: " << filePath;;
        return false;
    }
    OSRDEBUG << "INFO: Read in admin0 CTM file";
    ctmFreeContext(ctmContext);

    // build a list of indices for the GL_LINES primitive;
    // (0,0,0) vertices denote the start of a new linestring
    for(size_t i=0; i < listVx.size(); i++)
    {
        if((listVx[i].x == 0) &&
           (listVx[i].y == 0) &&
           (listVx[i].z == 0))
        {
            if(listIx.size() > 0)
            {   listIx.pop_back();   }

            i++;
            listIx.push_back(i);
            continue;
        }
        listIx.push_back(i);
        listIx.push_back(i);
    }
    return true;
}

std::string MapRenderer::readFileAsString(std::string const &fileName)
{
    std::ifstream ifs(fileName.c_str());
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
                         (std::istreambuf_iterator<char>()    ) );
    return content;
}

void MapRenderer::getFontList(std::vector<std::string> &listFonts)
{
    listFonts.clear();
    std::vector<DataSet*>::iterator dsIt;
    for(dsIt = m_listDataSets.begin();
        dsIt != m_listDataSets.end(); ++dsIt)
    {
        DataSet * dataSet = (*dsIt);
        for(size_t i=0; i < dataSet->listStyleConfigs.size(); i++)
        {
            std::vector<std::string> listFontsInStyle;
            dataSet->listStyleConfigs[i]->GetFontList(listFontsInStyle);

            for(size_t j=0; j < listFontsInStyle.size(); j++)
            {   listFonts.push_back(listFontsInStyle[j]);   }
        }
    }

    std::vector<std::string>::iterator sIt;
    std::sort(listFonts.begin(),listFonts.end());
    sIt = std::unique(listFonts.begin(),listFonts.end());

    listFonts.resize(sIt-listFonts.begin());
}

size_t MapRenderer::getMaxWayLayer()
{
    size_t maxWayLayer=0;
    std::vector<DataSet*>::iterator dsIt;
    for(dsIt = m_listDataSets.begin();
        dsIt != m_listDataSets.end(); ++dsIt)
    {
        DataSet * dataSet = (*dsIt);
        for(size_t i=0; i < dataSet->listStyleConfigs.size(); i++)   {
            maxWayLayer = std::max(maxWayLayer,
                dataSet->listStyleConfigs[i]->GetMaxWayLayer());
        }
    }
    return maxWayLayer;
}

size_t MapRenderer::getMaxAreaLayer()
{
    size_t maxAreaLayer=0;

    std::vector<DataSet*>::iterator dsIt;
    for(dsIt = m_listDataSets.begin();
        dsIt != m_listDataSets.end(); ++dsIt)
    {
        DataSet * dataSet = (*dsIt);
        for(size_t i=0; i < dataSet->listStyleConfigs.size(); i++)   {
            maxAreaLayer = std::max(maxAreaLayer,
                dataSet->listStyleConfigs[i]->GetMaxAreaLayer());
        }
    }

    return maxAreaLayer;
}

std::string MapRenderer::getTypeName(DataSet *dataSet,
                                     osmscout::TypeId typeId)
{
    osmscout::TypeInfo typeInfo =
        dataSet->GetTypeConfig()->GetTypeInfo(typeId);

    return typeInfo.GetName();
}

void MapRenderer::printVector(Vec3 const &myVector)
{
    OSRDEBUG << "### > " << myVector.x
             << " " << myVector.y
             << " " << myVector.z;
}

void MapRenderer::printLLA(const PointLLA &myPointLLA)
{
    OSRDEBUG << "### > " << myPointLLA.lon
             << " " << myPointLLA.lat
             << " " << myPointLLA.alt;
}

void MapRenderer::printCamera(Camera const &cam)
{
    OSRDEBUG << "CAMERA:";
    OSRDEBUG << "[orientation]";
    printVector(cam.eye);
    printVector(cam.up);
    printVector(cam.viewPt);

    OSRDEBUG << "[bounds]";
    OSRDEBUG << "ECEF TL,TR,BL,BR:";
    this->printVector(m_camera.exTL);
    this->printVector(m_camera.exTR);
    this->printVector(m_camera.exBR);
    this->printVector(m_camera.exBL);
    OSRDEBUG << "LLA  TL,TR,BL,BR:";
    this->printLLA(convECEFToLLA(m_camera.exTL));
    this->printLLA(convECEFToLLA(m_camera.exTR));
    this->printLLA(convECEFToLLA(m_camera.exBR));
    this->printLLA(convECEFToLLA(m_camera.exBL));
    OSRDEBUG << "Min. Enclosing Bounds (min,max):";
    GeoBounds bounds;
    std::vector<Vec3> listVx(4);
    listVx[0] = m_camera.exTL;
    listVx[1] = m_camera.exTR;
    listVx[3] = m_camera.exBR;
    listVx[2] = m_camera.exBL;
    Vec3 vxIn = (listVx[0]+listVx[1]+listVx[2]).ScaledBy(1.0/3.0);
    // TODO
}

}

