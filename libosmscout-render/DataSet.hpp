#ifndef OSMSCOUTRENDER_DATASET_HPP
#define OSMSCOUTRENDER_DATASET_HPP

// osmscout includes
#include <osmscout/Database.h>
#include <osmscout/ObjectRef.h>
#include <osmscout/Way.h>

// osmscout-render includes
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "RenderStyleConfig.hpp"

#ifdef USE_BOOST
    #include <boost/unordered_map.hpp>
    #define TYPE_UNORDERED_MAP boost::unordered::unordered_map
    #define TYPE_UNORDERED_SET boost::unordered::unordered_set
    #define TYPE_UNORDERED_MULTIMAP boost::unordered::unordered_multimap
#else
    #include <unordered_map>
    #define TYPE_UNORDERED_MAP std::unordered_map
    #define TYPE_UNORDERED_SET std::unordered_set
    #define TYPE_UNORDERED_MULTIMAP std::unordered_multimap
#endif

namespace osmsrender
{

// ========================================================================== //
// ========================================================================== //

struct PointLLA
{
    PointLLA() :
        lon(0),lat(0),alt(0) {}

    PointLLA(double myLat, double myLon) :
        lat(myLat),lon(myLon),alt(0) {}

    PointLLA(double myLat, double myLon, double myAlt) :
        lon(myLon),lat(myLat),alt(myAlt) {}

    double lon;
    double lat;
    double alt;
};

struct GeoBounds
{
    GeoBounds() :
        minLat(0),maxLat(0),
        minLon(0),maxLon(0)
    {}

    double minLat; double maxLat;
    double minLon; double maxLon;
};

struct Camera
{
    Camera() :
        fovY(0),
        aspectRatio(0),
        nearDist(0),
        farDist(0)
    {}

    PointLLA LLA;
    Vec3 eye;
    Vec3 viewPt;
    Vec3 up;

    double fovY;
    double aspectRatio;
    double nearDist;
    double farDist;

    Vec3 exTL;
    Vec3 exTR;
    Vec3 exBR;
    Vec3 exBL;
};

struct WayXSec
{
    osmscout::Id wayId;
    bool isUsed;
};

//double nmod(double a,double b)
//{   // modulo with proper negative
//    // number support
//    return a - b*floor(a/b);
//}

// ========================================================================== //
// ========================================================================== //

// notes:
// reminder to implement constructor, destructor
// and assignment operator if we start using
// pointers where memory is locally allocated
// with new (like the old style BuildingData)

// geomPtr points to the engine specific data
// structure that is used to render this node
// (such as a node in a scene graph)

struct NodeRenderData
{
    // geometry data
    osmscout::NodeRef           nodeRef;
    Vec3                        nodePosn;
    FillStyle const *     fillRenderStyle;
    SymbolStyle const *   symbolRenderStyle;

    // label data
    bool                        hasLabel;
    std::string                 nameLabel;
    LabelStyle const *    nameLabelRenderStyle;


    void *geomPtr;
};

struct WayRenderData
{
    // geometry data
    osmscout::WayRef        wayRef;
    size_t                  wayLayer;
    std::vector<Vec3>       listWayPoints;
//    std::vector<bool>       listSharedNodes;
    LineStyle const*        lineRenderStyle;
    bool                    isCoast;

    // label data
    bool                        hasLabel;
    std::string                 nameLabel;
    LabelStyle const *          nameLabelRenderStyle;

    //
    std::vector<std::vector<WayXSec*> > listIntersections;

    void *geomPtr;
};

struct AreaRenderData
{
    AreaRenderData() : buildingHeight(20) {}    // todo why does this def to 20?

    // geometry data
    osmscout::WayRef                    areaRef;
    size_t                              areaLayer;
    Vec3                                centerPoint;
    bool                                pathIsCCW;
    size_t                              lod;
    std::vector<Vec3>                   listOuterPoints;
    std::vector<std::vector<Vec3> >     listListInnerPoints;
    FillStyle const*              fillRenderStyle;

    bool                        isBuilding;
    double                      buildingHeight;

    // label data
    bool                        hasName;
    std::string                 nameLabel;
    LabelStyle const *    nameLabelRenderStyle;

    void *geomPtr;
};

struct RelAreaRenderData
{
    osmscout::RelationRef       relRef;
    std::vector<AreaRenderData> listAreaData;

    void *geomPtr;
};

struct RelWayRenderData
{
    osmscout::RelationRef       relRef;
    std::vector<WayRenderData>  listWayData;

    void *geomPtr;
};

// ========================================================================== //
// ========================================================================== //

enum IntersectionType
{
    XSEC_FALSE,
    XSEC_TRUE,
    XSEC_COINCIDENT,
    XSEC_PARALLEL
};

enum OutlineType
{
    OL_CENTER,
    OL_RIGHT,
    OL_LEFT
};

// ========================================================================== //
// ========================================================================== //

// typedefs
typedef std::pair<Vec2,Vec2> LineVec2;
typedef std::vector<osmscout::GroundTile*>                                      ListTilePtrs;
typedef std::pair<osmscout::NodeRef,size_t>                                     NodeRefAndLod;
typedef std::pair<osmscout::WayRef,size_t>                                      WayRefAndLod;
typedef std::pair<osmscout::RelationRef,size_t>                                 RelRefAndLod;
typedef std::vector<TYPE_UNORDERED_MAP<osmscout::Id,NodeRenderData> >           ListNodeDataByLod;
typedef std::vector<TYPE_UNORDERED_MAP<osmscout::Id,WayRenderData> >            ListWayDataByLod;
typedef std::vector<TYPE_UNORDERED_MAP<osmscout::Id,AreaRenderData> >           ListAreaDataByLod;
typedef std::vector<TYPE_UNORDERED_MAP<osmscout::Id,RelWayRenderData> >         ListRelWayDataByLod;
typedef std::vector<TYPE_UNORDERED_MAP<osmscout::Id,RelAreaRenderData> >        ListRelAreaDataByLod;
typedef std::vector<TYPE_UNORDERED_MAP<osmscout::Id,osmscout::NodeRef> >        ListNodeRefsByLod;
typedef std::vector<TYPE_UNORDERED_MAP<osmscout::Id,osmscout::WayRef> >         ListWayRefsByLod;
typedef std::vector<TYPE_UNORDERED_MAP<osmscout::Id,osmscout::WayRef> >         ListAreaRefsByLod;
typedef std::vector<TYPE_UNORDERED_MAP<osmscout::Id,osmscout::RelationRef> >    ListRelWayRefsByLod;
typedef std::vector<TYPE_UNORDERED_MAP<osmscout::Id,osmscout::RelationRef> >    ListRelAreaRefsByLod;
typedef TYPE_UNORDERED_MULTIMAP<osmscout::Id,WayXSec>                           ListSharedNodes;
typedef std::vector<ListSharedNodes>                                            ListSharedNodesByLod;
typedef TYPE_UNORDERED_MULTIMAP<osmscout::TypeId,osmscout::NodeRef>             ListNodesByType;
typedef TYPE_UNORDERED_MULTIMAP<osmscout::TypeId,osmscout::WayRef>              ListWaysByType;
typedef TYPE_UNORDERED_MULTIMAP<osmscout::TypeId,osmscout::WayRef>              ListAreasByType;
typedef TYPE_UNORDERED_MULTIMAP<osmscout::Id,osmscout::Id>                      ListIdsById;

// ========================================================================== //
// ========================================================================== //

class DataSet
{
public:
    virtual osmscout::TypeConfig const * GetTypeConfig() const = 0;

    virtual bool GetBoundingBox(double &minLat,double &minLon,
                                double &maxLat,double &maxLon) const = 0;

    bool GetObjects(std::vector<GeoBounds> const &listBounds,
                    osmscout::TypeSet const &typeSet,
                    std::vector<osmscout::NodeRef> &listNodeRefs,
                    std::vector<osmscout::WayRef> &listWayRefs,
                    std::vector<osmscout::WayRef> &listAreaRefs,
                    std::vector<osmscout::RelationRef> &listRelWayRefs,
                    std::vector<osmscout::RelationRef> &listRelAreaRefs)
    {
        for(size_t i=0; i < listBounds.size(); i++)
        {
            std::vector<osmscout::NodeRef>      lsNodeRefs;
            std::vector<osmscout::WayRef>       lsWayRefs;
            std::vector<osmscout::WayRef>       lsAreaRefs;
            std::vector<osmscout::RelationRef>  lsRelWayRefs;
            std::vector<osmscout::RelationRef>  lsRelAreaRefs;

            bool opOk = this->getObjects(listBounds[i].minLon,
                                         listBounds[i].minLat,
                                         listBounds[i].maxLon,
                                         listBounds[i].maxLat,
                                         typeSet,
                                         lsNodeRefs,
                                         lsWayRefs,
                                         lsAreaRefs,
                                         lsRelWayRefs,
                                         lsRelAreaRefs);

            if(!opOk)   {   return false;   }

            listNodeRefs.insert(listNodeRefs.end(),
                lsNodeRefs.begin(),lsNodeRefs.end());

            listWayRefs.insert(listWayRefs.end(),
                lsWayRefs.begin(),lsWayRefs.end());

            listAreaRefs.insert(listAreaRefs.end(),
                lsAreaRefs.begin(),lsAreaRefs.end());

            listRelWayRefs.insert(listRelWayRefs.end(),
                lsRelWayRefs.begin(),lsRelWayRefs.end());

            listRelAreaRefs.insert(listRelAreaRefs.end(),
                lsRelAreaRefs.begin(),lsRelAreaRefs.end());
        }

        return true;
    }

protected:
    virtual bool getObjects(double minLon, double minLat,
                            double maxLon, double maxLat,
                            const osmscout::TypeSet &typeSet,
                            std::vector<osmscout::NodeRef> &listNodeRefs,
                            std::vector<osmscout::WayRef> &listWayRefs,
                            std::vector<osmscout::WayRef> &listAreaRefs,
                            std::vector<osmscout::RelationRef> &listRelWayRefs,
                            std::vector<osmscout::RelationRef> &listRelAreaRefs) = 0;

public:
    osmscout::TagId tagName;
    osmscout::TagId tagRef;
    osmscout::TagId tagBuilding;
    osmscout::TagId tagHeight;

    ListNodeDataByLod    listNodeData;
    ListWayDataByLod     listWayData;
    ListAreaDataByLod    listAreaData;
    ListRelAreaDataByLod listRelAreaData;
    ListRelWayDataByLod  listRelWayData;

    //
    ListSharedNodesByLod listSharedNodes;

    // check for intersections <NodeId,WayXSec>
//    ListWayXsecByNode    listSharedNodes;

    // RenderStyleConfig
    std::vector<RenderStyleConfig*> listStyleConfigs;
};

// ========================================================================== //
// ========================================================================== //

class DataSetOSM : public DataSet
{
public:
    DataSetOSM(osmscout::Database const *db)
    {
        m_database = db;

        // get tags (todo what happens if the tag isnt there?)
        tagName       = m_database->GetTypeConfig()->tagName;
        tagRef        = m_database->GetTypeConfig()->tagRef;
        tagBuilding   = m_database->GetTypeConfig()->GetTagId("building");
        tagHeight     = m_database->GetTypeConfig()->GetTagId("height");
    }

    osmscout::TypeConfig const * GetTypeConfig() const
    {   return m_database->GetTypeConfig();   }

    bool GetBoundingBox(double &minLat, double &minLon,
                        double &maxLat, double &maxLon) const
    {
        return m_database->GetBoundingBox(minLat,minLon,maxLat,maxLon);
    }

private:
    bool getObjects(double minLon, double minLat,
                    double maxLon, double maxLat,
                    const osmscout::TypeSet &typeSet,
                    std::vector<osmscout::NodeRef> &listNodeRefs,
                    std::vector<osmscout::WayRef> &listWayRefs,
                    std::vector<osmscout::WayRef> &listAreaRefs,
                    std::vector<osmscout::RelationRef> &listRelWayRefs,
                    std::vector<osmscout::RelationRef> &listRelAreaRefs)
    {
        bool opOk = m_database->GetObjects(minLon,minLat,
                                           maxLon,maxLat,
                                           typeSet,
                                           listNodeRefs,
                                           listWayRefs,
                                           listAreaRefs,
                                           listRelWayRefs,
                                           listRelAreaRefs);
        return opOk;
    }

    osmscout::Database const * m_database;
};

// ========================================================================== //
// ========================================================================== //

class DataSetOSMCoast : public DataSet
{
public:
    DataSetOSMCoast(osmscout::Database const *db)
    {
        m_database = db;

        osmscout::TypeConfig const * typeConfig =
                m_database->GetTypeConfig();

        // get types
        m_typeCoast = typeConfig->GetTypeId("_tile_coastline");
        m_typeLand  = typeConfig->GetTypeId("_tile_land");
        m_typeSea   = typeConfig->GetTypeId("_tile_sea");

        // set default mag
        m_magDefault = osmscout::magRegion;
    }

    osmscout::TypeConfig const * GetTypeConfig() const
    {   return m_database->GetTypeConfig();   }

    bool GetBoundingBox(double &minLat, double &minLon,
                        double &maxLat, double &maxLon) const
    {
        return m_database->GetBoundingBox(minLat,minLon,maxLat,maxLon);
    }

private:

    size_t intlog2(size_t val)
    {   // warn:
        // returns 0 for an
        // input of 0!
        size_t ret = 0;
        while(val != 0)   {
            val >>= 1;
            ret++;
        }
        return ret;
    }

    size_t genCellId(size_t xAbs, size_t yAbs)
    {
        // we build a unique tile id as follows:
        // [XXXX][YYYY]
        // XXXX = 4 digits for abs x cell (max 8192)
        // YYYY = 4 digits for abs y cell (max 8192)

        // note: max val for uint32_t
        // 4,294,967,295

        size_t tileId = xAbs*10000 + yAbs;
        return tileId;
    }

    bool getObjects(double minLon, double minLat,
                    double maxLon, double maxLat,
                    const osmscout::TypeSet &typeSet,
                    std::vector<osmscout::NodeRef> &listNodeRefs,
                    std::vector<osmscout::WayRef> &listWayRefs,
                    std::vector<osmscout::WayRef> &listAreaRefs,
                    std::vector<osmscout::RelationRef> &listRelWayRefs,
                    std::vector<osmscout::RelationRef> &listRelAreaRefs)
    {
        if(typeSet.IsTypeSet(m_typeCoast) ||
           typeSet.IsTypeSet(m_typeLand))
        {
            std::list<osmscout::GroundTile> listTiles;
            bool opOk = m_database->GetGroundTiles(minLon,minLat,maxLon,maxLat,
                                                   m_magDefault,listTiles);
            if(!opOk)   {   return opOk;   }

            // note:
            // there can be 100s of tiles for each cell (xAbs,yAbs) and this
            // makes it difficult to generate uids and impractical for generating
            // geometry, so we merge all tiles belonging to a single cell

            TYPE_UNORDERED_MAP<size_t,ListTilePtrs> listTilesByCell;
            TYPE_UNORDERED_MAP<size_t,ListTilePtrs>::iterator cellIt,findIt;

            std::list<osmscout::GroundTile>::iterator tileIt;
            for(tileIt = listTiles.begin();
                tileIt != listTiles.end(); ++tileIt)
            {
                if(tileIt->coords.size() == 0)
                {   continue;   }

                size_t cellId = genCellId(tileIt->xAbs,tileIt->yAbs);

                // check to see if this cell exists already
                findIt = listTilesByCell.find(cellId);
                if(findIt == listTilesByCell.end())
                {   // the cell doesn't exist; add it
                    ListTilePtrs listTilePtrs;
                    listTilePtrs.push_back(&(*tileIt));

                    std::pair<size_t,ListTilePtrs> insData;
                    insData.first = cellId;
                    insData.second = listTilePtrs;
                    listTilesByCell.insert(insData);
                }
                else    // cell exists, just add tile
                {   findIt->second.push_back(&(*tileIt));   }
            }

            // convert tiles into way geometry
            double kMinLat,kMaxLat,kMinLon,kMaxLon;
            for(cellIt = listTilesByCell.begin();
                cellIt != listTilesByCell.end(); ++cellIt)
            {   // for every cell

                // create the way that will hold all the
                // coastlines for this cell
                osmscout::WayRef wayRef(new osmscout::Way);
                wayRef->SetId(cellIt->first);
                wayRef->SetType(m_typeCoast);
                wayRef->SetStartIsJoint(true);
                wayRef->SetEndIsJoint(true);

                osmscout::Point vx;
                ListTilePtrs &listTilePtrs = cellIt->second;
                for(size_t i=0; i < listTilePtrs.size(); i++)
                {   // for every tile
                    osmscout::GroundTile * tilePtr = listTilePtrs[i];
                    kMinLat = tilePtr->yAbs*tilePtr->cellHeight-90.0;
                    kMaxLat = kMinLat + tilePtr->cellHeight;
                    kMinLon = tilePtr->xAbs*tilePtr->cellWidth-180.0;
                    kMaxLon = kMinLon + tilePtr->cellWidth;

                    size_t lineStart = 0;
                    size_t lineEnd;

                    while(lineStart < tilePtr->coords.size())
                    {
                        // seek lineStart to start of coastline segment
                        while(lineStart < tilePtr->coords.size() &&
                              !(tilePtr->coords[lineStart].coast))
                        {   lineStart++;   }

                        if(lineStart >= tilePtr->coords.size())
                        {   continue;   }

                        // seek lineEnd to end of coastline segment
                        lineEnd = lineStart;
                        while(lineEnd < tilePtr->coords.size() &&
                              tilePtr->coords[lineEnd].coast)
                        {   lineEnd++;   }

                        for(size_t n=lineStart; n <= lineEnd; n++)
                        {
                            double lon = kMinLon+tilePtr->coords[n].x*tilePtr->cellWidth/
                                    osmscout::GroundTile::Coord::CELL_MAX;

                            double lat = kMinLat+tilePtr->coords[n].y*tilePtr->cellHeight/
                                    osmscout::GroundTile::Coord::CELL_MAX;

                            vx.Set(lat,lon); wayRef->nodes.push_back(vx);
                        }
                        lineStart = lineEnd+1;
                        vx.Set(0,0); wayRef->nodes.push_back(vx);
                    }
                }
                // mark the end of the coastline data and save
                wayRef->nodes.pop_back();
                listWayRefs.push_back(wayRef);
            }
        }
        return true;
    }

    osmscout::Database const * m_database;
    osmscout::TypeId m_typeCoast;
    osmscout::TypeId m_typeLand;
    osmscout::TypeId m_typeSea;
    osmscout::Mag m_magDefault;
};

// ========================================================================== //
// ========================================================================== //

class DataSetTemp : public DataSet
{
public:
    DataSetTemp(osmscout::TypeConfig * typeConfig) :
        m_minLat(90.0),m_minLon(180.0),
        m_maxLat(-90.0),m_maxLon(-180.0),
        m_typeConfig(typeConfig),
        m_id_counter(0)
    {
        // setup tags
        tagName     = typeConfig->tagName;
        tagBuilding = typeConfig->GetTagId("building");
        tagHeight   = typeConfig->GetTagId("height");

        if(tagBuilding == osmscout::typeIgnore)
        {   tagBuilding = typeConfig->RegisterTagForExternalUse("building");   }

        if(tagHeight == osmscout::typeIgnore)
        {   tagHeight = typeConfig->RegisterTagForExternalUse("height");   }
    }

    ~DataSetTemp();

    bool AddNode(osmscout::Node const &addNode,
                 std::vector<osmscout::Tag> listTags,
                 size_t &nodeId)
    {
        osmscout::TypeInfo nodeTypeInfo =
                m_typeConfig->GetTypeInfo(addNode.GetType());

        if((nodeTypeInfo.GetId() != osmscout::typeIgnore) &&
            nodeTypeInfo.CanBeNode())
        {
            // copy node data
            osmscout::NodeRef nodeRef(new osmscout::Node);
            nodeRef->SetId(genObjectId());
            nodeRef->SetType(addNode.GetType());
            nodeRef->SetCoordinates(addNode.GetLon(),addNode.GetLat());
            nodeRef->SetTags(listTags);

            // save to list
            std::pair<osmscout::TypeId,osmscout::NodeRef> insData;
            insData.first = nodeRef->GetType();
            insData.second = nodeRef;

            m_listNodesByType.insert(insData);
            resizeBoundingBox(addNode.GetLat(),addNode.GetLon());

            // save id
            nodeId = nodeRef->GetId();
            return true;
        }
        return false;
    }

    bool AddWay(osmscout::Way const &addWay,
                std::vector<osmscout::Tag> &listTags,
                size_t &wayId)
    {
        osmscout::TypeInfo wayTypeInfo =
                m_typeConfig->GetTypeInfo(addWay.GetType());

        if((wayTypeInfo.GetId() != osmscout::typeIgnore) &&
            (wayTypeInfo.CanBeWay()))
        {
            // copy way data
            osmscout::WayRef wayRef(new osmscout::Way);
            wayRef->SetId(genObjectId());
            wayRef->SetType(addWay.GetType());
            wayRef->SetStartIsJoint(addWay.StartIsJoint());
            wayRef->SetEndIsJoint(addWay.EndIsJoint());
            wayRef->nodes = addWay.nodes;

            osmscout::SilentProgress segAttProgress;
            bool reverseNodes = false;
            wayRef->SetTags(segAttProgress,
                            *(this->GetTypeConfig()),
                            false,listTags,reverseNodes);

            if(reverseNodes)   {
                std::reverse(wayRef->nodes.begin(),
                             wayRef->nodes.end());
            }

            // save to list
            std::pair<osmscout::TypeId,osmscout::WayRef> insData;
            insData.first = wayRef->GetType();
            insData.second = wayRef;

            m_listWaysByType.insert(insData);

            // save id
            wayId = wayRef->GetId();
            return true;
        }
        return false;
    }

    bool AddArea(osmscout::Way const &addArea,
                 std::vector<osmscout::Tag> &listTags,
                 size_t &areaId)
    {
        osmscout::TypeInfo areaTypeInfo =
                m_typeConfig->GetTypeInfo(addArea.GetType());

        if((areaTypeInfo.GetId() != osmscout::typeIgnore) &&
           (areaTypeInfo.CanBeArea()))
        {
            // copy way data
            osmscout::WayRef areaRef(new osmscout::Way);
            areaRef->SetId(genObjectId());
            areaRef->SetType(addArea.GetType());
            areaRef->SetStartIsJoint(addArea.StartIsJoint());
            areaRef->SetEndIsJoint(addArea.EndIsJoint());
            areaRef->nodes = addArea.nodes;

            osmscout::SilentProgress segAttProgress;
            bool reverseNodes = false;
            areaRef->SetTags(segAttProgress,
                             *(this->GetTypeConfig()),
                             false,listTags,reverseNodes);

            if(reverseNodes)   {
                std::reverse(areaRef->nodes.begin(),
                             areaRef->nodes.end());
            }

            // save to list
            std::pair<osmscout::TypeId,osmscout::WayRef> insData;
            insData.first = areaRef->GetType();
            insData.second = areaRef;

            m_listAreasByType.insert(insData);

            // save id
            areaId = areaRef->GetId();
            return true;
        }
        return false;
    }

    // note: findRange.first == findRange.last == end()
    // when no search results are found

    bool RemoveNode(size_t const nodeId)
    {
        ListNodesByType::iterator nIt;
        for(nIt = m_listNodesByType.begin();
            nIt != m_listNodesByType.end(); ++nIt)
        {
            if(nIt->second->GetId() == nodeId)   {
                m_listNodesByType.erase(nIt);
                return true;
            }
        }
        return false;
    }

    bool RemoveWay(size_t const wayId)
    {
        ListWaysByType::iterator wIt;
        for(wIt = m_listWaysByType.begin();
            wIt != m_listWaysByType.end(); ++wIt)
        {
            if(wIt->second->GetId() == wayId)   {
                m_listWaysByType.erase(wIt);
                return true;
            }
        }
        return false;
    }


    bool RemoveArea(size_t const areaId)
    {
        ListAreasByType::iterator aIt;
        for(aIt = m_listAreasByType.begin();
            aIt != m_listAreasByType.end(); ++aIt)
        {
            if(aIt->second->GetId() == areaId)   {
                m_listAreasByType.erase(aIt);
                return true;
            }
        }
        return true;
    }

    osmscout::TypeConfig const * GetTypeConfig() const
    {   return m_typeConfig;   }

    bool GetBoundingBox(double &minLat, double &minLon,
                        double &maxLat, double &maxLon) const
    {   // TODO FIX ME
        minLat = m_minLat;
        minLon = m_minLon;
        maxLat = m_maxLat;
        maxLon = m_maxLon;
        return true;
    }

private:
    bool getObjects(double minLon, double minLat,
                    double maxLon, double maxLat,
                    const osmscout::TypeSet &typeSet,
                    std::vector<osmscout::NodeRef> &listNodeRefs,
                    std::vector<osmscout::WayRef> &listWayRefs,
                    std::vector<osmscout::WayRef> &listAreaRefs,
                    std::vector<osmscout::RelationRef> &listRelWayRefs,
                    std::vector<osmscout::RelationRef> &listRelAreaRefs)
    {


        // get types we query objects for
        std::vector<osmscout::TypeId> listQueryTypes;
        std::vector<osmscout::TypeInfo> listTypeInfo = m_typeConfig->GetTypes();
        for(size_t i=0; i < listTypeInfo.size(); i++)   {
            if(typeSet.IsTypeSet(listTypeInfo[i].GetId()))   {
                listQueryTypes.push_back(listTypeInfo[i].GetId());
            }
        }

        for(size_t i=0; i < listQueryTypes.size(); i++)
        {
            // [nodes]
            ListNodesByType::const_iterator nIt;
            std::pair<ListNodesByType::const_iterator,
                      ListNodesByType::const_iterator> nodeRange;
            nodeRange = m_listNodesByType.equal_range(listQueryTypes[i]);

            for(nIt = nodeRange.first;  nIt != nodeRange.second; ++nIt)
            {   // save to list if within bounds
                if(isWithinBounds(minLat,minLon,maxLat,maxLon,
                                  nIt->second->GetLat(),
                                  nIt->second->GetLon()))
                {   listNodeRefs.push_back(nIt->second);   }
            }

            // [ways]
            ListWaysByType::const_iterator wIt;
            std::pair<ListWaysByType::const_iterator,
                      ListWaysByType::const_iterator> wayRange;
            wayRange = m_listWaysByType.equal_range(listQueryTypes[i]);

            for(wIt = wayRange.first; wIt != wayRange.second; ++wIt)
            {
                double centerLat,centerLon;
                wIt->second->GetCenter(centerLat,centerLon);
                if(isWithinBounds(minLat,minLon,maxLat,maxLon,
                                  centerLat,centerLon))
                {   listWayRefs.push_back(wIt->second);   }
            }

            // [areas]
            ListAreasByType::const_iterator aIt;
            std::pair<ListAreasByType::const_iterator,
                      ListAreasByType::const_iterator> areaRange;
            areaRange = m_listAreasByType.equal_range(listQueryTypes[i]);

            for(aIt = areaRange.first; aIt != areaRange.second; ++aIt)
            {
                double centerLat,centerLon;
                aIt->second->GetCenter(centerLat,centerLon);
                if(isWithinBounds(minLat,minLon,maxLat,maxLon,
                                  centerLat,centerLon))
                {   listAreaRefs.push_back(aIt->second);    }
            }

            // [relation ways] (unsupported)
            // [relation areas] (unsupported)
        }
        return true;
    }

    size_t genObjectId()
    {
        if(m_id_counter > 32767)   {    // randomly using 16-bit as upper limit;
            m_id_counter = 0;           // we should never have this many objects
        }
        else   {
            m_id_counter++;
        }

        return m_id_counter;
    }

    void resizeBoundingBox(double objLat,double objLon)
    {
        m_minLat = std::min(objLat,m_minLat);
        m_minLon = std::min(objLon,m_minLon);
        m_maxLat = std::max(objLat,m_maxLat);
        m_maxLon = std::max(objLon,m_maxLon);
    }

    bool isWithinBounds(double minLat,double minLon,
                        double maxLat,double maxLon,
                        double objLat,double objLon) const
    {
        if((objLat <= maxLat) && (objLat >= minLat))   {
            if((objLon <= maxLon) && (objLon >= minLon))   {
                return true;
            }
        }
        return false;
    }

    size_t m_id_counter;
    double m_minLat;
    double m_minLon;
    double m_maxLat;
    double m_maxLon;
    osmscout::TypeConfig const * m_typeConfig;
    ListNodesByType     m_listNodesByType;
    ListWaysByType      m_listWaysByType;
    ListAreasByType     m_listAreasByType;
};

}

#endif // OSMSCOUTRENDER_DATASET_HPP
