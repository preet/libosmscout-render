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


namespace osmscout
{

MapRenderer::MapRenderer(Database const *myDatabase) :
    m_database(myDatabase),m_dataMinLat(0),m_dataMinLon(0),
    m_dataMaxLon(0),m_dataMaxLat(0)
{
    m_wayNodeCount = 0; // todo: is this working right? do I reset the value correctly?

    //
    m_tagName = m_database->GetTypeConfig()->tagName;
    m_tagBuilding = m_database->GetTypeConfig()->GetTagId("building");
    m_tagHeight = m_database->GetTypeConfig()->GetTagId("height");
}

MapRenderer::~MapRenderer()
{}

// ========================================================================== //
// ========================================================================== //

void MapRenderer::SetRenderStyleConfigs(const std::vector<RenderStyleConfig*> &listStyleConfigs)
{
    // clear old render style configs and save new ones
    m_listRenderStyleConfigs.clear();
    m_listRenderStyleConfigs.resize(listStyleConfigs.size());

    for(int i=0; i < listStyleConfigs.size(); i++)
    {   m_listRenderStyleConfigs[i] = listStyleConfigs[i];   }

    // a new list of render style configs invalidates all
    // current scene data, so we clear everything
    removeAllFromScene();
    m_listNodeData.clear();
    m_listWayData.clear();
    m_listAreaData.clear();
    m_listRelAreaData.clear();
    m_listSharedNodes.clear();

    // rebuild all styleConfig related data
    m_listNodeData.resize(listStyleConfigs.size());
    m_listWayData.resize(listStyleConfigs.size());
    m_listAreaData.resize(listStyleConfigs.size());
    m_listRelAreaData.resize(listStyleConfigs.size());

    for(int i=0; i < listStyleConfigs.size(); i++)  {
        m_listNodeData[i].reserve(200);
        m_listWayData[i].reserve(350);
        m_listAreaData[i].reserve(200);
        m_listRelAreaData[i].reserve(50);

    }
    m_listSharedNodes.reserve(5000);

    rebuildStyleData(listStyleConfigs);
}

void MapRenderer::GetDebugLog(std::vector<std::string> &listDebugMessages)
{
    for(int i=0; i < m_listMessages.size(); i++)
    {   listDebugMessages.push_back(m_listMessages.at(i));   }
}

// ========================================================================== //
// ========================================================================== //

void MapRenderer::InitializeScene()
{
    if(m_listRenderStyleConfigs.empty())
    {   OSRDEBUG << "ERROR: No render style configs specified!";   return;   }

    // call implementation
    initScene();

    // set camera / update scene
    double minLat, minLon, maxLat, maxLon;
    m_database->GetBoundingBox(minLat,minLon,maxLat,maxLon);
    PointLLA camLLA((minLat+maxLat)/2,(minLon+maxLon)/2,500.0);

    SetCamera(camLLA,30.0,1.67);
}

void MapRenderer::InitializeScene(const PointLLA &camLLA,
                                  double fovy, double aspectRatio)
{
    if(m_listRenderStyleConfigs.empty())
    {   OSRDEBUG << "ERROR: No render style configs specified!";   return;   }

    // call virtual implementation
    initScene();

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

    if(!calcCameraViewExtents())
    {
//        m_camera.nearDist = 20;
//        m_camera.farDist = ELL_SEMI_MAJOR*1.25;
        OSRDEBUG << "WARN: Could not calculate view extents";
    }
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
    if(!calcCameraViewExtents())
    {
//        m_camera.nearDist = 20;
//        m_camera.farDist = ELL_SEMI_MAJOR*1.25;
        OSRDEBUG << "WARN: Could not calculate view extents";
    }
    else
    {   updateSceneBasedOnCamera();   }
}

Camera const * MapRenderer::GetCamera()
{   return &m_camera;   }

// ========================================================================== //
// ========================================================================== //

void MapRenderer::updateSceneContents()
{
    // calculate the minimum and maximum distance to
    // m_camera.eye within the available lat/lon bounds
    Vec3 viewBoundsNE;   convLLAToECEF(PointLLA(m_camera.maxLat,m_camera.maxLon),viewBoundsNE);
    Vec3 viewBoundsNW;   convLLAToECEF(PointLLA(m_camera.maxLat,m_camera.minLon),viewBoundsNW);
    Vec3 viewBoundsSW;   convLLAToECEF(PointLLA(m_camera.minLat,m_camera.minLon),viewBoundsSW);
    Vec3 viewBoundsSE;   convLLAToECEF(PointLLA(m_camera.minLat,m_camera.maxLon),viewBoundsSE);

    // to get the minimum distance, find the minima
    // of the minimum distances between m_camera.eye and
    // each edge of the bounding box
    double minDistToNEdge = calcMinPointLineDistance(m_camera.eye,viewBoundsNE,viewBoundsNW);
    double minDistToWEdge = calcMinPointLineDistance(m_camera.eye,viewBoundsNW,viewBoundsSW);
    double minDistToSEdge = calcMinPointLineDistance(m_camera.eye,viewBoundsSW,viewBoundsSE);
    double minDistToEEdge = calcMinPointLineDistance(m_camera.eye,viewBoundsNE,viewBoundsSE);

    double minDistToViewBounds = minDistToNEdge;

    if(minDistToWEdge < minDistToViewBounds)
    {   minDistToViewBounds = minDistToWEdge;   }

    if(minDistToSEdge < minDistToViewBounds)
    {   minDistToViewBounds = minDistToSEdge;   }

    if(minDistToEEdge < minDistToViewBounds)
    {   minDistToViewBounds = minDistToEEdge;   }

    // to get the maximum distance, find the maxima
    // of the distances to each corner of the bounding box
    double distToNE,distToNW,distToSE,distToSW;
    distToNE = m_camera.eye.DistanceTo(viewBoundsNE);
    distToNW = m_camera.eye.DistanceTo(viewBoundsNW);
    distToSE = m_camera.eye.DistanceTo(viewBoundsSE);
    distToSW = m_camera.eye.DistanceTo(viewBoundsSW);

    double maxDistToViewBounds = distToNE;

    if(distToNW > maxDistToViewBounds)
    {   maxDistToViewBounds = distToNW;   }

    if(distToSE > maxDistToViewBounds)
    {   maxDistToViewBounds = distToSE;   }

    if(distToSW > maxDistToViewBounds)
    {   maxDistToViewBounds = distToSW;   }

//    OSRDEBUG << "INFO: minDistToViewBounds: " << minDistToViewBounds;
//    OSRDEBUG << "INFO: maxDistToViewBounds: " << maxDistToViewBounds;

    // use the min and max distance between m_camera.eye
    // and the view bounds to set active LOD ranges
    unsigned int numLodRanges = m_listRenderStyleConfigs.size();
    std::vector<bool> listLODRangesActive(numLodRanges);
    std::vector<std::pair<double,double> > listLODRanges(numLodRanges);

    for(int i=0; i < numLodRanges; i++)
    {
        std::pair<double,double> lodRange;
        lodRange.first = m_listRenderStyleConfigs.at(i)->GetMinDistance();
        lodRange.second = m_listRenderStyleConfigs.at(i)->GetMaxDistance();
        listLODRanges[i] = lodRange;

        // if the min-max distance range overlaps with
        // lodRange, set the range as active
        if(lodRange.second < minDistToViewBounds ||
           lodRange.first > maxDistToViewBounds)
        {   listLODRangesActive[i] = false;   }
        else
        {   listLODRangesActive[i] = true;   }
    }

    // check if at least one valid style
    bool hasValidStyle = false;
    for(int i=0; i < listLODRangesActive.size(); i++)
    {
        if(listLODRangesActive[i])
        {   hasValidStyle = true;   break;   }
    }

    if(!hasValidStyle)
    {   OSRDEBUG << "WARN: No valid style data found";   return;   }

    // for all ranges that are active, get the overlap
    // of the range extents with the view extents to
    // define a bounding box for the database query
    PointLLA camLLA;
    convECEFToLLA(m_camera.eye,camLLA);

    unsigned int numRanges = listLODRanges.size();
    std::vector<TYPE_UNORDERED_MAP<Id,NodeRef> >  listNodeRefsByLod(numRanges);
    std::vector<TYPE_UNORDERED_MAP<Id,WayRef> >   listWayRefsByLod(numRanges);
    std::vector<TYPE_UNORDERED_MAP<Id,WayRef> >   listAreaRefsByLod(numRanges);
    std::vector<TYPE_UNORDERED_MAP<Id,RelationRef> > listRelWayRefsByLod(numRanges);
    std::vector<TYPE_UNORDERED_MAP<Id,RelationRef> > listRelAreaRefsByLod(numRanges);

    TYPE_UNORDERED_MAP<Id,NodeRefAndLod> listNodeRefsAllLods(300);
    TYPE_UNORDERED_MAP<Id,WayRefAndLod>  listWayRefsAllLods(600);
    TYPE_UNORDERED_MAP<Id,WayRefAndLod>  listAreaRefsAllLods(300);
    TYPE_UNORDERED_MAP<Id,RelRefAndLod>  listRelWayRefsAllLods(50);
    TYPE_UNORDERED_MAP<Id,RelRefAndLod>  listRelAreaRefsAllLods(100);

    for(int i=0; i < numRanges; i++)
    {
        if(listLODRangesActive[i])
        {
            // get range extents based on camera and lodRange
            PointLLA rangeN,rangeE,rangeS,rangeW;
            calcGeographicDestination(camLLA,0,listLODRanges[i].second,rangeN);
            calcGeographicDestination(camLLA,90,listLODRanges[i].second,rangeE);
            calcGeographicDestination(camLLA,180,listLODRanges[i].second,rangeS);
            calcGeographicDestination(camLLA,270,listLODRanges[i].second,rangeW);

            // check if range and view extents intersect
            if((m_camera.maxLon < rangeW.lon) || (m_camera.minLon > rangeE.lon) ||
               (m_camera.maxLat < rangeS.lat) || (m_camera.minLat > rangeN.lat))
            {   continue;   }

            // get intersection rectangle
            double queryMinLon = std::max(m_camera.minLon,rangeW.lon);
            double queryMinLat = std::max(m_camera.minLat,rangeS.lat);
            double queryMaxLon = std::min(m_camera.maxLon,rangeE.lon);
            double queryMaxLat = std::min(m_camera.maxLat,rangeN.lat);
//            OSRDEBUG << "queryMinLat: " << queryMinLat;
//            OSRDEBUG << "queryMaxLat: " << queryMaxLat;
//            OSRDEBUG << "queryMinLon: " << queryMinLon;
//            OSRDEBUG << "queryMaxLon: " << queryMaxLon;

            // get objects from database
            TypeSet typeSet;
            m_listRenderStyleConfigs[i]->GetActiveTypes(typeSet);

            std::vector<NodeRef>        listNodeRefs;
            std::vector<WayRef>         listWayRefs;
            std::vector<WayRef>         listAreaRefs;
            std::vector<RelationRef>    listRelWayRefs;
            std::vector<RelationRef>    listRelAreaRefs;

            if(m_database->GetObjects(queryMinLon,queryMinLat,
                                      queryMaxLon,queryMaxLat,
                                      typeSet,
                                      listNodeRefs,
                                      listWayRefs,
                                      listAreaRefs,
                                      listRelWayRefs,
                                      listRelAreaRefs))
            {
                // we retrieve objects from a high LOD (close up zoom)
                // to a lower LOD (far away zoom)

                // since the database query does not have finite resolution,
                // we cull all results that have already been retrieved for
                // all previous LOD ranges to prevent duplicates by first
                // inserting into a 'parent' map<Id,[]RefAndLod> before
                // saving into the 'sub' map<Id,[]Ref> organzied by lod

                // note: since some types can be shared in the definition file,
                // we need to exclusively keep track of which sets of nodes, ways
                // areas should be kept -- so even if the db query returns certain
                // primitives, we only use them if they are explicitly specified

                // NODES
                std::vector<NodeRef>::iterator nodeIt;
                for(nodeIt = listNodeRefs.begin();
                    nodeIt != listNodeRefs.end();)
                {
                    if(m_listRenderStyleConfigs[i]->GetNodeTypeIsValid((*nodeIt)->GetType()))
                    {
                        // note: for some reason, libosmscout returns a large number
                        // of nodes, well beyond the specified bounds in the database
                        // GetObjects() call, so we only use nodes inside the bounds
                        double myLat = (*nodeIt)->GetLat();
                        double myLon = (*nodeIt)->GetLon();
                        if(myLat >= queryMinLat && myLat <= queryMaxLat &&
                           myLon >= queryMinLon && myLon <= queryMaxLon)
                        {
                            NodeRefAndLod nodeRefLod(*nodeIt,i);
                            std::pair<Id,NodeRefAndLod> insNode((*nodeIt)->GetId(),nodeRefLod);

                            if(listNodeRefsAllLods.insert(insNode).second)
                            {   listNodeRefsByLod[i].insert(std::make_pair((*nodeIt)->GetId(),*nodeIt));   }
                        }
                    }
                    ++nodeIt;
                }

                // WAYS
                std::vector<WayRef>::iterator wayIt;
                for(wayIt = listWayRefs.begin();
                    wayIt != listWayRefs.end();)
                {
                    if(m_listRenderStyleConfigs[i]->GetWayTypeIsValid((*wayIt)->GetType()))
                    {
                        WayRefAndLod wayRefLod(*wayIt,i);
                        std::pair<Id,WayRefAndLod> insWay((*wayIt)->GetId(),wayRefLod);

                        if(listWayRefsAllLods.insert(insWay).second)
                        {   listWayRefsByLod[i].insert(std::make_pair((*wayIt)->GetId(),*wayIt));   }
                    }
                    ++wayIt;
                }

                // AREAS
                std::vector<WayRef>::iterator areaIt;
                for(areaIt = listAreaRefs.begin();
                    areaIt != listAreaRefs.end();)
                {
                    if(m_listRenderStyleConfigs[i]->GetAreaTypeIsValid((*areaIt)->GetType()))
                    {
                        WayRefAndLod areaRefLod(*areaIt,i);
                        std::pair<Id,WayRefAndLod> insArea((*areaIt)->GetId(),areaRefLod);

                        if(listAreaRefsAllLods.insert(insArea).second)
                        {   listAreaRefsByLod[i].insert(std::make_pair((*areaIt)->GetId(),*areaIt));   }
                    }
                    ++areaIt;
                }

                // RELATION AREAS
                std::vector<RelationRef>::iterator relIt;
                for(relIt = listRelAreaRefs.begin();
                    relIt != listRelAreaRefs.end();)
                {
                    if(m_listRenderStyleConfigs[i]->GetAreaTypeIsValid((*relIt)->GetType()))
                    {
                        RelRefAndLod relRefLod(*relIt,i);
                        std::pair<Id,RelRefAndLod> insRel((*relIt)->GetId(),relRefLod);

                        if(listRelAreaRefsAllLods.insert(insRel).second)
                        {   listRelAreaRefsByLod[i].insert(std::make_pair((*relIt)->GetId(),*relIt));   }
                    }
                    ++relIt;
                }
            }
        }
    }

    updateNodeRenderData(listNodeRefsByLod);
    updateWayRenderData(listWayRefsByLod);
    updateAreaRenderData(listAreaRefsByLod);
    updateRelAreaRenderData(listRelAreaRefsByLod);

    // todo ... call this after or before
    // 'update current data extents'?
    this->doneUpdatingAreas();
    this->doneUpdatingRelAreas();

    // update current data extents
    m_dataMinLat = m_camera.minLat;
    m_dataMinLon = m_camera.minLon;
    m_dataMaxLat = m_camera.maxLat;
    m_dataMaxLon = m_camera.maxLon;
}

void MapRenderer::updateSceneBasedOnCamera()
{
    double oldArea = (m_dataMaxLat-m_dataMinLat)*
                     (m_dataMaxLon-m_dataMinLon);

    double newArea = (m_camera.maxLat-m_camera.minLat)*
                     (m_camera.maxLon-m_camera.minLon);

    double overlapArea = calcAreaRectOverlap(m_camera.minLon,m_camera.minLat,
                                             m_camera.maxLon,m_camera.maxLat,
                                             m_dataMinLon,m_dataMinLat,
                                             m_dataMaxLon,m_dataMaxLat);

    if(oldArea < 1E-7)
    {   updateSceneContents();   return;   }

    double oldOverlap = overlapArea/oldArea;
    double newOverlap = overlapArea/newArea;

    if(oldOverlap < 0.75 || newOverlap < 0.75)
    {
        // just for debugging
        showCameraViewArea(m_camera);

        // update scene contents
        OSRDEBUG << "INFO: [Updating Scene Contents...]";
        updateSceneContents();
    }
}

bool MapRenderer::calcCameraViewExtents()
{
    return calcCameraViewExtents(m_camera.eye,m_camera.viewPt,m_camera.up,
                                 m_camera.fovY,m_camera.aspectRatio,
                                 m_camera.nearDist,m_camera.farDist,
                                 m_camera.minLat,m_camera.maxLat,
                                 m_camera.minLon,m_camera.maxLon);
}

// ========================================================================== //
// ========================================================================== //

void MapRenderer::updateNodeRenderData(std::vector<TYPE_UNORDERED_MAP<Id,NodeRef> > &listNodeRefsByLod)
{
    unsigned int thingsAdded=0;
    unsigned int thingsRemoved=0;

    for(int i=0; i < listNodeRefsByLod.size(); i++)
    {
        TYPE_UNORDERED_MAP<Id,NodeRef>::iterator itNew;
        TYPE_UNORDERED_MAP<Id,NodeRenderData>::iterator itOld;

        // remove objects from the old view extents
        // not present in the new view extents
        for(itOld = m_listNodeData[i].begin();
            itOld != m_listNodeData[i].end();)
        {
            itNew = listNodeRefsByLod[i].find((*itOld).first);

            if(itNew == listNodeRefsByLod[i].end())
            {   // node dne in new view -- remove it
                TYPE_UNORDERED_MAP<Id,NodeRenderData>::iterator itDelete = itOld;
                removeNodeFromScene((*itDelete).second); ++itOld;
                m_listNodeData[i].erase(itDelete);
                thingsRemoved++;
            }
            else
            {   ++itOld;   }
        }

        // add objects from the new view extents
        // not present in the old view extents
        NodeRenderData nodeRenderData;
        for(itNew = listNodeRefsByLod[i].begin();
            itNew != listNodeRefsByLod[i].end(); ++itNew)
        {
            itOld = m_listNodeData[i].find((*itNew).first);

            if(itOld == m_listNodeData[i].end())
            {   // node dne in old view -- add it
                if(genNodeRenderData((*itNew).second,
                                     m_listRenderStyleConfigs[i],
                                     nodeRenderData))
                {
                    addNodeToScene(nodeRenderData);
                    std::pair<Id,NodeRenderData> insPair((*itNew).first,nodeRenderData);
                    m_listNodeData[i].insert(insPair);
                    thingsAdded++;
                }
            }
        }
    }
//        OSRDEBUG << "INFO:    Nodes Removed: " << thingsRemoved;
//        OSRDEBUG << "INFO:    Nodes Added: " << thingsAdded;
}

void MapRenderer::updateWayRenderData(std::vector<TYPE_UNORDERED_MAP<Id,WayRef> > &listWayRefsByLod)
{
    int thingsAdded = 0;
    int thingsRemoved = 0;

//    OSRDEBUG << "INFO:    Shared Nodes Before Update: " << m_listSharedNodes.size();

    for(int i=0; i < listWayRefsByLod.size(); i++)
    {
        TYPE_UNORDERED_MAP<Id,WayRef>::iterator itNew;
        TYPE_UNORDERED_MAP<Id,WayRenderData>::iterator itOld;

        // remove objects from the old view extents
        // not present in the new view extents
        for(itOld = m_listWayData[i].begin();
            itOld != m_listWayData[i].end();)
        {
            itNew = listWayRefsByLod[i].find((*itOld).first);

            if(itNew == listWayRefsByLod[i].end())
            {   // way dne in new view -- remove it
                TYPE_UNORDERED_MAP<Id,WayRenderData>::iterator itDelete = itOld;

                removeWayFromSharedNodes(itDelete->second.wayRef);
                removeWayFromScene((*itDelete).second); ++itOld;
                m_listWayData[i].erase(itDelete);
                thingsRemoved++;
            }
            else
            {   ++itOld;   }
        }

        // add objects from the new view extents
        // not present in the old view extents
        WayRenderData wayRenderData;
        for(itNew = listWayRefsByLod[i].begin();
            itNew != listWayRefsByLod[i].end(); ++itNew)
        {
            itOld = m_listWayData[i].find((*itNew).first);

            if(itOld == m_listWayData[i].end())
            {   // way dne in old view -- add it
                if(genWayRenderData((*itNew).second,
                                    m_listRenderStyleConfigs[i],
                                    wayRenderData))
                {
                    addWayToScene(wayRenderData);
                    std::pair<Id,WayRenderData> insPair((*itNew).first,wayRenderData);
                    m_listWayData[i].insert(insPair);
                    thingsAdded++;
                }
            }
        }
    }

//    OSRDEBUG << "INFO:    Ways Removed: " << thingsRemoved;
//    OSRDEBUG << "INFO:    Ways Added: " << thingsAdded;
//    OSRDEBUG << "INFO:    Shared Nodes After Update: " << m_listSharedNodes.size();
}

void MapRenderer::updateAreaRenderData(std::vector<TYPE_UNORDERED_MAP<Id,WayRef> > &listAreaRefsByLod)
{
    for(int i=0; i < listAreaRefsByLod.size(); i++)
    {
        TYPE_UNORDERED_MAP<Id,WayRef>::iterator itNew;
        TYPE_UNORDERED_MAP<Id,AreaRenderData>::iterator itOld;

        // remove objects from the old view extents
        // not present in the new view extents
        for(itOld = m_listAreaData[i].begin();
            itOld != m_listAreaData[i].end();)
        {
            itNew = listAreaRefsByLod[i].find((*itOld).first);

            if(itNew == listAreaRefsByLod[i].end())
            {   // way dne in new view -- remove it
                TYPE_UNORDERED_MAP<Id,AreaRenderData>::iterator itDelete = itOld;
                removeAreaFromScene((*itDelete).second); ++itOld;
                m_listAreaData[i].erase(itDelete);
            }
            else
            {   ++itOld;   }
        }

        // add objects from the new view extents
        // not present in the old view extents
        for(itNew = listAreaRefsByLod[i].begin();
            itNew != listAreaRefsByLod[i].end(); ++itNew)
        {
            itOld = m_listAreaData[i].find((*itNew).first);

            if(itOld == m_listAreaData[i].end())
            {   // way dne in old view -- add it
                AreaRenderData areaRenderData;
                areaRenderData.lod = i;

                if(genAreaRenderData((*itNew).second,
                                     m_listRenderStyleConfigs[i],
                                     areaRenderData))
                {
                    addAreaToScene(areaRenderData);
                    std::pair<Id,AreaRenderData> insPair((*itNew).first,areaRenderData);
                    m_listAreaData[i].insert(insPair);
                }
            }
        }
    }
}

void MapRenderer::updateRelAreaRenderData(std::vector<TYPE_UNORDERED_MAP<Id,RelationRef> > &listRelRefsByLod)
{
    for(int i=0; i < listRelRefsByLod.size(); i++)
    {
        TYPE_UNORDERED_MAP<Id,RelationRef>::iterator itNew;
        TYPE_UNORDERED_MAP<Id,RelAreaRenderData>::iterator itOld;

        // remove objects from the old view extents
        // not present in the new view extents
        for(itOld = m_listRelAreaData[i].begin();
            itOld != m_listRelAreaData[i].end();)
        {
            itNew = listRelRefsByLod[i].find((*itOld).first);

            if(itNew == listRelRefsByLod[i].end())
            {   // way dne in new view -- remove it
                TYPE_UNORDERED_MAP<Id,RelAreaRenderData>::iterator itDelete = itOld;
                removeRelAreaFromScene((*itDelete).second); ++itOld;
                m_listRelAreaData[i].erase(itDelete);
            }
            else
            {   ++itOld;   }
        }

        // add objects from the new view extents
        // not present in the old view extents
        for(itNew = listRelRefsByLod[i].begin();
            itNew != listRelRefsByLod[i].end(); ++itNew)
        {
            itOld = m_listRelAreaData[i].find((*itNew).first);

            if(itOld == m_listRelAreaData[i].end())
            {   // way dne in old view -- add it
                RelAreaRenderData relRenderData;

                if(genRelAreaRenderData((*itNew).second,
                                        m_listRenderStyleConfigs[i],
                                        relRenderData))
                {
                    addRelAreaToScene(relRenderData);
                    std::pair<Id,RelAreaRenderData> insPair((*itNew).first,relRenderData);
                    m_listRelAreaData[i].insert(insPair);
                }
            }
        }
    }
}

// ========================================================================== //
// ========================================================================== //

bool MapRenderer::genNodeRenderData(const NodeRef &nodeRef,
                                    const RenderStyleConfig *renderStyle,
                                    NodeRenderData &nodeRenderData)
{
    TypeId nodeType = nodeRef->GetType();

    nodeRenderData.nodeRef = nodeRef;
    nodeRenderData.fillRenderStyle =
            renderStyle->GetNodeFillRenderStyle(nodeType);
    nodeRenderData.symbolRenderStyle =
            renderStyle->GetNodeSymbolRenderStyle(nodeType);

    // get node geometry
    nodeRenderData.nodePosn =
            convLLAToECEF(PointLLA(nodeRef->GetLat(),
                                   nodeRef->GetLon()));
    // node label data
    std::string nameLabel;
    for(size_t i=0; i < nodeRef->GetTagCount(); i++)  {
        if(nodeRef->GetTagKey(i) == m_tagName)
        {   nameLabel = nodeRef->GetTagValue(i);   }
    }

    nodeRenderData.nameLabel = nameLabel;
    nodeRenderData.nameLabelRenderStyle =
            renderStyle->GetNodeNameLabelRenderStyle(nodeType);
    nodeRenderData.hasName = (nodeRenderData.nameLabel.size() > 0) &&
            !(nodeRenderData.nameLabelRenderStyle == NULL);

    return true;
}

bool MapRenderer::genWayRenderData(const WayRef &wayRef,
                                   const RenderStyleConfig *renderStyle,
                                   WayRenderData &wayRenderData)
{
    TypeId wayType = wayRef->GetType();

    // set general way properties
    wayRenderData.wayRef = wayRef;
    wayRenderData.wayLayer = renderStyle->GetWayLayer(wayType);
    wayRenderData.lineRenderStyle =
            renderStyle->GetWayLineRenderStyle(wayType);

    // build way geometry
    wayRenderData.listWayPoints.resize(wayRef->nodes.size());
    this->getListOfSharedWayNodes(wayRef,wayRenderData.listSharedNodes);

    for(int i=0; i < wayRef->nodes.size(); i++)
    {
        wayRenderData.listWayPoints[i] =
                convLLAToECEF(PointLLA(wayRef->nodes[i].GetLat(),
                                       wayRef->nodes[i].GetLon(),0.0));

        std::pair<Id,Id> nodeInWay(wayRef->nodes[i].GetId(),wayRef->GetId());
        m_listSharedNodes.insert(nodeInWay);

        m_wayNodeCount++;
    }

    // way label data
    wayRenderData.nameLabel = wayRef->GetName();
    wayRenderData.nameLabelRenderStyle =
            renderStyle->GetWayNameLabelRenderStyle(wayType);
    wayRenderData.hasName = (wayRenderData.nameLabel.size() > 0) &&
            !(wayRenderData.nameLabelRenderStyle == NULL);

    return true;
}

bool MapRenderer::genRelWayRenderData(const RelationRef &relRef,
                                      const RenderStyleConfig *renderStyle,
                                      RelWayRenderData &relRenderData)
{}

bool MapRenderer::genAreaRenderData(const WayRef &areaRef,
                                    const RenderStyleConfig *renderStyle,
                                    AreaRenderData &areaRenderData)
{
    // ensure that the area is valid before building
    // the area geometry in ecef coordinates
    double minLat = 200;
    double minLon = 200;
    double maxLat = -200;
    double maxLon = -200;
    std::vector<osmscout::Vec2> listOuterPoints(areaRef->nodes.size());
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

    TypeId areaType = areaRef->GetType();

    // check if area is a building
    areaRenderData.isBuilding = false;
    double areaHeight = 0;
    if(areaRef->GetTagCount() > 0)
    {
        for(int i=0; i < areaRef->GetTagCount(); i++)
        {
            if(areaRef->GetTagKey(i) == m_tagBuilding)
            {
                std::string keyVal = areaRef->GetTagValue(i);
                if(keyVal != "no" && keyVal != "false" && keyVal != "0")
                {   areaRenderData.isBuilding = true;   }
            }

            else if(areaRef->GetTagKey(i) == m_tagHeight)
            {   areaHeight = convStrToDbl(areaRef->GetTagValue(i));   }
        }
    }

    // set area data
    areaRenderData.areaRef = areaRef;
    areaRenderData.areaLayer =
            renderStyle->GetAreaLayer(areaType);
    areaRenderData.fillRenderStyle =
            renderStyle->GetAreaFillRenderStyle(areaType);

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
            renderStyle->GetAreaNameLabelRenderStyle(areaType);
    areaRenderData.hasName = (areaRenderData.nameLabel.size() > 0) &&
            !(areaRenderData.nameLabelRenderStyle == NULL);

    //


    return true;
}

bool MapRenderer::genRelAreaRenderData(const RelationRef &relRef,
                                       const RenderStyleConfig *renderStyle,
                                       RelAreaRenderData &relRenderData)
{
//    // debug
//    // this check shouldn't be necessary since we
//    // explicitly request types we want
//    if(relRef->GetType() == osmscout::typeIgnore)
//    {   return false;   }

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
        TypeId areaType;

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

        std::vector<osmscout::Vec2>                 listOuterPts;
        std::vector<std::vector<osmscout::Vec2> >   listListInnerPts;

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

            osmscout::Vec2 myPt(myLon,myLat);
            listOuterPts.push_back(myPt);
        }

        // save inner ring nodes
        for(int j=0; j < listDirectChildren[i].size(); j++)
        {
            std::vector<osmscout::Vec2> listInnerPts;
            unsigned int chIdx = listDirectChildren[i][j];
            for(int v=0; v < relRef->roles[chIdx].nodes.size(); v++)
            {
                osmscout::Vec2 myPt(relRef->roles[chIdx].nodes[v].GetLon(),
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
                if(relRef->GetTagKey(j) == m_tagBuilding)
                {
                    std::string keyVal = relRef->GetTagValue(j);
                    if(keyVal != "no" && keyVal != "no" && keyVal != "0")
                    {   areaData.isBuilding = true;   }
                }
                else if(relRef->GetTagKey(j) == m_tagHeight)
                {   areaHeight = convStrToDbl(relRef->GetTagValue(j));   }
            }

            if(!areaData.isBuilding)
            {
                for(int j=0; j < relRef->roles[i].attributes.tags.size(); j++)
                {
                    Tag myTag = relRef->roles[i].attributes.tags[j];
                    if(myTag.key == m_tagBuilding)
                    {
                        std::string keyVal = myTag.value;
                        if(keyVal != "no" && keyVal != "no" && keyVal != "0")
                        {   areaData.isBuilding = true;   }
                    }
                    else if(myTag.key == m_tagHeight)
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
                Tag myTag = relRef->roles[i].attributes.tags[j];
                if(myTag.key == m_tagBuilding)
                {
                    std::string keyVal = myTag.value;
                    if(keyVal != "no" && keyVal != "no" && keyVal != "0")
                    {   areaData.isBuilding = true;   }
                }
                else if(myTag.key == m_tagHeight)
                {   areaHeight = convStrToDbl(myTag.value);   }
            }

            // get area name using attributes
            areaData.nameLabel = relRef->roles[i].attributes.name;
        }

        // set area properties/materials
        areaData.areaLayer = renderStyle->GetAreaLayer(areaType);
        areaData.fillRenderStyle = renderStyle->GetAreaFillRenderStyle(areaType);

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
                renderStyle->GetAreaNameLabelRenderStyle(areaType);
        areaData.hasName = (areaData.nameLabel.size() > 0) &&
                !(areaData.nameLabelRenderStyle == NULL);

        // save
        relRenderData.listAreaData.push_back(areaData);
    }

    relRenderData.relRef = relRef;

//    // debug multipolyon ring hierarchy
//    OSRDEBUG << "Relation Area ID: " << relRef->GetId();
//    OSRDEBUG << "Relation Type: " << this->getTypeName(relRef->GetId());
//    for(size_t i = 0; i < relRef->roles.size(); i++)   {
//        OSRDEBUG << "Role " << i
//                 << ", Ring " << int(relRef->roles[i].ring)
//                 << ", Type " << relRef->roles[i].GetType();
//    }

    return true;
}

// ========================================================================== //
// ========================================================================== //

void MapRenderer::getListOfSharedWayNodes(const WayRef &wayRef,
                                          std::vector<bool> &listSharedNodes)
{
    listSharedNodes.resize(wayRef->nodes.size());

    for(int i=0; i < wayRef->nodes.size(); i++)
    {
        size_t waysSharedByNode =
                m_listSharedNodes.count(wayRef->nodes[i].GetId());

        if(waysSharedByNode > 0)
        {
            if(waysSharedByNode > 1)
            {   listSharedNodes[i] = true;   }
            else
            {   // if waysSharedByNode == 1, we have to check
                // whether or not the way that this node belongs
                // to isn't already the given way
                Id nodeId = wayRef->nodes[i].GetId();
                Id wayId = wayRef->GetId();

                if(m_listSharedNodes.find(nodeId)->second == wayId)
                {   listSharedNodes[i] = false;   }
                else
                {   listSharedNodes[i] = true;   }
            }
        }
        else
        {   listSharedNodes[i] = false;   }
    }
}

void MapRenderer::removeWayFromSharedNodes(const WayRef &wayRef)
{
    TYPE_UNORDERED_MULTIMAP<Id,Id>::iterator itShNode;
    std::pair<TYPE_UNORDERED_MULTIMAP<Id,Id>::iterator,
              TYPE_UNORDERED_MULTIMAP<Id,Id>::iterator> itRange;

    for(int i=0; i < wayRef->nodes.size(); i++)
    {
        itRange = m_listSharedNodes.equal_range(wayRef->nodes[i].GetId());

        for(itShNode = itRange.first;
            itShNode != itRange.second;)
        {
            if(itShNode->second == wayRef->GetId())
            {   m_listSharedNodes.erase(itShNode);  break;   }

            ++itShNode;
        }
    }
}

void MapRenderer::clearAllRenderData()
{
    for(int i=0; i < m_listRenderStyleConfigs.size(); i++)
    {
        m_listWayData[i].clear();
        m_listAreaData[i].clear();
    }

    removeAllFromScene();
}

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

//bool MapRenderer::calcPolyIsSimple(const std::vector<Vec2> &listPolyPoints)
//{
//    // test poly by starting with a given edge, and
//    // checking whether or not it intersects with any
//    // other edges in the polygon

//    // this is done naively O(n^2) -- better
//    // implementation would be a line sweep algorithm

//    std::vector<Vec2> listPoints(listPolyPoints.size()+1);
//    for(int i=0; i < listPoints.size()-1; i++)
//    {   listPoints[i] = listPolyPoints[i];   }
//    listPoints.back() = listPolyPoints[0];

//    for(int i=0; i < listPoints.size()-1; i++)
//    {
//        unsigned int edgesIntersect = 0;
//        for(int j=i+1; j < listPoints.size()-1; j++)
//        {
//            if(calcLinesIntersect(listPoints[i].x,
//                                  listPoints[i].y,
//                                  listPoints[i+1].x,
//                                  listPoints[i+1].y,
//                                  listPoints[j].x,
//                                  listPoints[j].y,
//                                  listPoints[j+1].x,
//                                  listPoints[j+1].y))
//            {
//                edgesIntersect++;

//                // when i == 0, we check the first edge against every
//                // other edge and expect to see 2 intersections for
//                // adjacent edges; poly is complex if there are more
//                // intersections

//                if(i == 0)
//                {
//                    if(edgesIntersect > 2)
//                    {   return false;   }
//                }

//                // when i != 0 we check an edge that isn't the first
//                // edge against every other edge excluding those that
//                // have already been tested (this means one adjacent
//                // edge); poly is complex if there is more than one
//                // intersection

//                else
//                {
//                    if(edgesIntersect > 1)
//                    {   return false;   }
//                }
//            }
//        }
//    }
//    return true;
//}

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

double MapRenderer::calcAreaRectOverlap(double r1_bl_x, double r1_bl_y,
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

bool MapRenderer::calcGeographicDestination(const PointLLA &pointStart,
                                            double bearingDegrees,
                                            double distanceMeters,
                                            PointLLA &pointDest)
{
    // ref: http://www.movable-type.co.uk/scripts/latlong.html

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

bool MapRenderer::calcCameraViewExtents(const Vec3 &camEye,
                                        const Vec3 &camViewpoint,
                                        const Vec3 &camUp,
                                        const double &camFovY,
                                        const double &camAspectRatio,
                                        double &camNearDist, double &camFarDist,
                                        double &camMinLat, double &camMaxLat,
                                        double &camMinLon, double &camMaxLon)
{
    Vec3 camAlongViewpoint = camViewpoint-camEye;

    // calculate four edge vectors of the frustum
    double camFovY_rad_bi = (camFovY*K_PI/180.0)/2;
    double dAlongViewpoint = cos(camFovY_rad_bi);
    double dAlongUp = sin(camFovY_rad_bi);
    double dAlongRight = dAlongUp*camAspectRatio;

    Vec3 ecefCenter;
    Vec3 camRight = camAlongViewpoint.Cross(camUp);
    Vec3 vAlongViewpoint = camAlongViewpoint.Normalized().ScaledBy(dAlongViewpoint);
    Vec3 vAlongUp = camUp.Normalized().ScaledBy(dAlongUp);
    Vec3 vAlongRight = camRight.Normalized().ScaledBy(dAlongRight);

    Vec3 viewTL = vAlongViewpoint + vAlongUp - vAlongRight;
    Vec3 viewTR = vAlongViewpoint + vAlongUp + vAlongRight;
    Vec3 viewBL = vAlongViewpoint - vAlongUp - vAlongRight;
    Vec3 viewBR = vAlongViewpoint - vAlongUp + vAlongRight;

    std::vector<Vec3> listFrustumEdgeVectors(4);
    listFrustumEdgeVectors[0] = viewTL;
    listFrustumEdgeVectors[1] = viewTR;
    listFrustumEdgeVectors[2] = viewBL;
    listFrustumEdgeVectors[3] = viewBR;

    // determine the camera parameters based on which
    // frustum edge vectors intersect with the Earth
    std::vector<bool> listIntersectsEarth(4);
    std::vector<Vec3> listIntersectionPoints(4);
    bool allIntersect = true;   // indicates all camera vectors intersect Earth's surface
    bool noneIntersect = true;  // indicates no camera vectors intersect Earth's surface

    for(int i=0; i < listFrustumEdgeVectors.size(); i++)
    {
        listIntersectsEarth[i] =
                calcRayEarthIntersection(camEye,
                                         listFrustumEdgeVectors[i],
                                         listIntersectionPoints[i]);

        if(!listIntersectsEarth[i])
        {
            // if any frustum vectors do not intersect Earth's surface,
            // intersect the vectors with a plane through Earth's
            // center that is normal to the camera's view direction

            // (the corresponding POI represents the horizon at some
            // arbitrary height -- we ignore altitude data anyway)

            bool intersectsPlane =
                calcRayPlaneIntersection(camEye, listFrustumEdgeVectors[i],
                                         ecefCenter, camAlongViewpoint,
                                         listIntersectionPoints[i]);

            // if the any of the camera vectors do not intersect the
            // center plane, assume the camera is invalid
            if(!intersectsPlane)
            {   return false;   }
        }

        allIntersect = allIntersect && listIntersectsEarth[i];
        noneIntersect = noneIntersect && !listIntersectsEarth[i];

//        OSRDEBUG << "INFO: POI (" << listIntersectionPoints[i].x
//                 << "  " << listIntersectionPoints[i].y
//                 << "  " << listIntersectionPoints[i].z << ")";
    }

    if(allIntersect)
    {
        // set the near clipping plane distance to be
        // 1/3 the distance between camEye and the Earth
        Vec3 earthSurfacePoint;
        calcRayEarthIntersection(camEye,
                                 camAlongViewpoint,
                                 earthSurfacePoint);

        camNearDist = camEye.DistanceTo(earthSurfacePoint)*(3.0/4.0);
    }
    else
    {
        // set near clipping plane to 1/3 of the minimum
        // distance between camEye and view frustum
        // intersection points (with Earth or center plane)
        double minDist = 0;
        std::vector<double> listIntersectionDist2s(4);
        for(int i=0; i < listFrustumEdgeVectors.size(); i++)
        {
            listIntersectionDist2s[i] =
                    camEye.Distance2To(listIntersectionPoints[i]);

            if(i == 0)
            {   minDist = listIntersectionDist2s[i];   }
            else
            {
                if(listIntersectionDist2s[i] < minDist)
                {   minDist = listIntersectionDist2s[i];   }
            }
        }

        camNearDist = sqrt(minDist)*(3.0/4.0);
    }

    // set the far clipping plane to be the distance
    // between the camera eye and a plane through Earth's
    // center that is normal to the camera's view direction
    // (represents distance between camEye and horizon)
    camFarDist = calcMinPointPlaneDistance(camEye,ecefCenter,
                                           camAlongViewpoint);

    // find and save view extents in LLA
    std::vector<PointLLA> listPointLLA(4);
    convECEFToLLA(listIntersectionPoints[0],listPointLLA[0]);
    convECEFToLLA(listIntersectionPoints[1],listPointLLA[1]);
    convECEFToLLA(listIntersectionPoints[2],listPointLLA[2]);
    convECEFToLLA(listIntersectionPoints[3],listPointLLA[3]);

    for(int i=0; i < 4; i++)
    {   convECEFToLLA(listIntersectionPoints[i],listPointLLA[i]);   }

    camMinLat = 95; camMaxLat = -95;
    camMinLon = 185; camMaxLon = -185;
    for(int i=0; i < 4; i++)
    {
        PointLLA const &xsecPt = listPointLLA[i];
        camMinLat = std::min(camMinLat,xsecPt.lat);
        camMinLon = std::min(camMinLon,xsecPt.lon);
        camMaxLat = std::max(camMaxLat,xsecPt.lat);
        camMaxLon = std::max(camMaxLon,xsecPt.lon);
    }

    return true;
}

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

std::string MapRenderer::readFileAsString(std::string const &fileName)
{
    std::ifstream ifs(fileName.c_str());
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
                         (std::istreambuf_iterator<char>()    ) );
    return content;
}

TypeConfig const * MapRenderer::getTypeConfig()
{   return m_database->GetTypeConfig();   }

void MapRenderer::getFontList(std::vector<std::string> &listFonts)
{
    std::vector<std::string> listFontsInStyle;
    std::vector<std::string>::iterator it;

    for(int i=0; i < m_listRenderStyleConfigs.size(); i++)
    {
        m_listRenderStyleConfigs[i]->GetFontList(listFontsInStyle);

        for(int j=0; j < listFontsInStyle.size(); j++)
        {   listFonts.push_back(listFontsInStyle[j]);   }
    }

    std::sort(listFonts.begin(),listFonts.end());
    it = std::unique(listFonts.begin(),listFonts.end());

    listFonts.resize(it-listFonts.begin());
}

size_t MapRenderer::getMaxWayLayer()
{
    size_t maxWayLayer=0;
    for(int i=0; i < m_listRenderStyleConfigs.size(); i++)
    {
        maxWayLayer = std::max(maxWayLayer,
            m_listRenderStyleConfigs[i]->GetMaxWayLayer());
    }

    return maxWayLayer;
}

size_t MapRenderer::getMaxAreaLayer()
{
    size_t maxAreaLayer=0;
    for(int i=0; i < m_listRenderStyleConfigs.size(); i++)
    {
        maxAreaLayer = std::max(maxAreaLayer,
            m_listRenderStyleConfigs[i]->GetMaxAreaLayer());
    }

    return maxAreaLayer;
}

std::string MapRenderer::getTypeName(TypeId typeId)
{
    TypeInfo typeInfo = m_database->GetTypeConfig()->GetTypeInfo(typeId);
    return typeInfo.GetName();
}

void MapRenderer::printVector(Vec3 const &myVector)
{
    OSRDEBUG << "#>" << myVector.x << "," << myVector.y << "," << myVector.z;
}

}

