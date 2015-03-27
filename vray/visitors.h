#ifndef __abc_vray_visitors_h__
#define __abc_vray_visitors_h__

#include "common.h"
#include "userattr.h"
#include "geomsrc.h"
#include "plugin.h"

class BaseVisitor
{
public:
   
   BaseVisitor(AlembicGeometrySource *geosrc);
   virtual ~BaseVisitor();
   
   bool getVisibility(Alembic::Abc::ICompoundProperty props, double t);
   
   template <class T>
   bool isVisible(AlembicNodeT<T> &node)
   {
      if (mGeoSrc->params()->ignoreVisibility)
      {
         return true;
      }
      else
      {
         return getVisibility(node.object().getProperties(), mGeoSrc->renderTime());
      }
   }
   
   inline bool isVisible(AlembicNode *node)
   {
      if (mGeoSrc->params()->ignoreVisibility)
      {
         return true;
      }
      else
      {
         return getVisibility(node->object().getProperties(), mGeoSrc->renderTime());
      }
   }
   
protected:
   
   AlembicGeometrySource *mGeoSrc;
};

// ---

class BuildPlugins : public BaseVisitor
{
public:
   
   BuildPlugins(AlembicGeometrySource *geosrc,
                VR::VRayRenderer *vray);
   virtual ~BuildPlugins();
   
   AlembicNode::VisitReturn enter(AlembicXform &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicMesh &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicSubD &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicPoints &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicCurves &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicNuPatch &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicNode &node, AlembicNode *instance=0);
      
   void leave(AlembicNode &node, AlembicNode *instance=0);

protected:
   
   VR::VRayRenderer *mVRay;
};

// ---

class CollectWorldMatrices : public BaseVisitor
{
public:
   
   CollectWorldMatrices(AlembicGeometrySource *geosrc);
   
   AlembicNode::VisitReturn enter(AlembicXform &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicMesh &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicSubD &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicPoints &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicCurves &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicNuPatch &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicNode &node, AlembicNode *instance=0);
   
   void leave(AlembicXform &node, AlembicNode *instance=0);
   void leave(AlembicNode &node, AlembicNode *instance=0);
   
   inline const std::map<std::string, Alembic::Abc::M44d>& getWorldMatrices() const
   {
      return mMatrices;
   }
   
   inline bool getWorldMatrix(const std::string &objectPath, Alembic::Abc::M44d &W) const
   {
      std::map<std::string, Alembic::Abc::M44d>::const_iterator it = mMatrices.find(objectPath);
      if (it != mMatrices.end())
      {
         W = it->second;
         return true;
      }
      else
      {
         return false;
      }
   }
   
private:
   
   template <class T>
   AlembicNode::VisitReturn shapeEnter(AlembicNodeT<T> &node, AlembicNode *instance=0);
   
private:
   
   std::vector<Alembic::Abc::M44d> mMatrixStack;
   std::map<std::string, Alembic::Abc::M44d> mMatrices;
};

template <class T>
AlembicNode::VisitReturn CollectWorldMatrices::shapeEnter(AlembicNodeT<T> &node, AlembicNode *instance)
{
   Alembic::Abc::M44d W;
   
   if (mMatrixStack.size() > 0)
   {
      W = mMatrixStack.back();
   }
   
   std::string path = (instance ? instance->path() : node.path());
   
   mMatrices[path] = W;
   
   return AlembicNode::ContinueVisit;
}

// ---

class UpdateGeometry : public BaseVisitor
{
public:
   
   UpdateGeometry(AlembicGeometrySource *geosrc);
   virtual ~UpdateGeometry();
   
   AlembicNode::VisitReturn enter(AlembicXform &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicMesh &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicSubD &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicPoints &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicCurves &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicNuPatch &node, AlembicNode *instance=0);
   AlembicNode::VisitReturn enter(AlembicNode &node, AlembicNode *instance=0);
   
   void leave(AlembicXform &node, AlembicNode *instance=0);   
   void leave(AlembicNode &node, AlembicNode *instance=0);
   
   
public:
   
   struct UserAttrsSet
   {
      UserAttributes object;
      UserAttributes primitive;
      UserAttributes point;
      UserAttributes vertex;
      std::map<std::string, Alembic::AbcGeom::IV2fGeomParam> uvs;
   };
   
   void collectUserAttributes(Alembic::Abc::ICompoundProperty userProps,
                              Alembic::Abc::ICompoundProperty geomParams,
                              double t,
                              bool interpolate,
                              UserAttrsSet &attrs,
                              bool object,
                              bool primitive,
                              bool point,
                              bool vertex,
                              bool uvs);
   
   template <class MeshSchema>
   bool readBaseMesh(AlembicNodeT<Alembic::Abc::ISchemaObject<MeshSchema> > &node,
                     AbcVRayGeom *geom,
                     const UserAttrsSet &attrs,
                     bool computeNormals);
   
   template <class MeshSchema>
   inline bool readBaseMesh(AlembicNodeT<Alembic::Abc::ISchemaObject<MeshSchema> > &node,
                            AbcVRayGeom *geom,
                            bool computeNormals)
   {
      UserAttrsSet attrs;
      return readBaseMesh(node, geom, attrs, computeNormals);
   }
   
   float* computeMeshSmoothNormals(AbcVRayGeom *geom,
                                   const float *P0,
                                   const float *P1,
                                   float blend);
   
   void readMeshNormals(AlembicMesh &mesh,
                        AbcVRayGeom *geom);
   
   template <class MeshSchema>
   void setMeshSmoothNormals(AlembicNodeT<Alembic::Abc::ISchemaObject<MeshSchema> > &node,
                             AbcVRayGeom *geom);
   
   template <class MeshSchema>
   size_t readMeshUVs(AlembicNodeT<Alembic::Abc::ISchemaObject<MeshSchema> > &node,
                      AbcVRayGeom *geom,
                      const UserAttrsSet &attrs,
                      bool primaryOnly=false);
   
   template <class MeshSchema>
   inline size_t readMeshUVs(AlembicNodeT<Alembic::Abc::ISchemaObject<MeshSchema> > &node,
                             AbcVRayGeom *geom)
   {
      UserAttrsSet attrs;
      return readMeshUVs(node, geom, attrs, true);
   }
   
protected:
   
   std::vector<std::vector<Alembic::Abc::M44d> > mMatrixSamplesStack;
};

template <class MeshSchema>
bool UpdateGeometry::readBaseMesh(AlembicNodeT<Alembic::Abc::ISchemaObject<MeshSchema> > &node,
                                  AbcVRayGeom *geom,
                                  const UserAttrsSet &attrs,
                                  bool computeNormals)
{
   MeshSchema &schema = node.typedObject().getSchema();
   
   bool isConst = (geom->constPositions != 0);
   bool varyingTopology = schema.getTopologyVariance() == Alembic::AbcGeom::kHeterogenousTopology;
   bool singleSample = (mGeoSrc->params()->ignoreDeformBlur || varyingTopology);
   double renderTime = mGeoSrc->renderTime();
   double renderFrame = mGeoSrc->renderFrame();
   double *times = (singleSample ? &renderTime : mGeoSrc->sampleTimes());
   double *frames = (singleSample ? &renderFrame : mGeoSrc->sampleFrames());
   size_t ntimes = (singleSample ? 1 : mGeoSrc->numTimeSamples());
   double t = 0.0;
   double f = 0.0;
   double b = 0.0;
   bool rv = true;
   
   
   if (mGeoSrc->params()->verbose && varyingTopology)
   {
      std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: Topology varying mesh" << std::endl;
   }
   
   // Sample mesh schema
   
   TimeSampleList<MeshSchema> &meshSamples = node.samples().schemaSamples;
   typename TimeSampleList<MeshSchema>::ConstIterator samp0, samp1; // for animated mesh
   typename MeshSchema::Sample theSample; // for reference mesh
   
   // Build topology
   Alembic::Abc::Int32ArraySamplePtr FC;
   Alembic::Abc::Int32ArraySamplePtr FI;
   
   if (isConst)
   {
      if (mGeoSrc->params()->verbose)
      {
         std::cout << "[AlembicLoader] UpdateGeometr::readBaseMesh: Constant geometry" << std::endl;
      }
      theSample = schema.getValue();
      
      FC = theSample.getFaceCounts();
      FI = theSample.getFaceIndices();
   }
   else
   {
      // Sample mesh schema
      for (size_t i=0; i<ntimes; ++i)
      {
         t = times[i];
         node.sampleSchema(t, t, i > 0);
      }
      
      if (meshSamples.size() == 0)
      {
         return false;
      }
      
      meshSamples.getSamples(renderTime, samp0, samp1, b);
      
      FC = samp0->data().getFaceCounts();
      FI = samp0->data().getFaceIndices();
      
      if (mGeoSrc->params()->verbose)
      {
         std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: Use topology at t=" << samp0->time() << std::endl;
      }
   }
   
   if (!FC->valid() || !FI->valid())
   {
      std::cerr << "[AlembicLoader] UpdateGeometry::readBaseMesh: Invalid topology data" << std::endl;
      return false;
   }
   
   if (mGeoSrc->params()->verbose)
   {
      std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: " << FC->size() << " face(s), " << FI->size() << " vertice(s)" << std::endl;
   }
      
   // Compute number of triangles
   
   geom->numTriangles = 0;
   
   for (size_t i=0; i<FC->size(); ++i)
   {
      unsigned int nv = FC->get()[i];
      
      if (nv > 3)
      {
         geom->numTriangles += nv - 2;
      }
      else
      {
         geom->numTriangles += (nv == 3 ? 1 : 0);
      }
   }
   
   geom->faces->setCount(geom->numTriangles * 3, renderFrame);
   
   geom->numFaces = FC->size();
   geom->numFaceVertices = FI->size();
   
   if (mGeoSrc->params()->verbose)
   {
      std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: " << geom->numTriangles << " triangle(s)" << std::endl;
   }
   
   // Alembic to V-Ray indices mappings
   geom->toVertexIndex = new unsigned int[geom->numTriangles * 3];
   geom->toPointIndex = new unsigned int[geom->numTriangles * 3];
   geom->toFaceIndex = new unsigned int[geom->numTriangles * 3];
   
   int edgeVisMask = 0;
   int edgeVisMaskIdx = 0;
   int edgeVisCnt = (geom->numTriangles / 10) + ((geom->numTriangles % 10) > 0 ? 1 : 0);
   
   if (mGeoSrc->params()->verbose)
   {
      std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: " << edgeVisCnt << " edge visibility flag(s)" << std::endl;
   }
   
   geom->edgeVisibility->setCount(edgeVisCnt, renderFrame);
   
   edgeVisCnt = 0;
   
   // i: face index
   // j: index in FI array
   // k: index in geom->faces array
   for (size_t i=0, j=0, k=0; i<FC->size(); ++i)
   {
      // Alembic winding order is clockwise, V-Ray expects counter-clockwise
      unsigned int nv = FC->get()[i];
      
      if (nv >= 3)
      {
         // First vertex in CCW order
         unsigned int j0 = j + nv - 1;
         unsigned int i0 = FI->get()[j0];
         
         // For each triangle (simple faning)
         for (unsigned int l=nv-2; l>=1; --l)
         {
            unsigned int j1 = j + l;
            unsigned int i1 = FI->get()[j1];
            
            unsigned int j2 = j + l - 1;
            unsigned int i2 = FI->get()[j2];
            
            geom->faces->setInt(i0, k, renderFrame);
            geom->toFaceIndex[k] = i;
            geom->toPointIndex[k] = i0;
            geom->toVertexIndex[k] = j0;
            ++k;
            
            geom->faces->setInt(i1, k, renderFrame);
            geom->toFaceIndex[k] = i;
            geom->toPointIndex[k] = i1;
            geom->toVertexIndex[k] = j1;
            ++k;
            
            geom->faces->setInt(i2, k, renderFrame);
            geom->toFaceIndex[k] = i;
            geom->toPointIndex[k] = i2;
            geom->toVertexIndex[k] = j2;
            ++k;
            
            // first edge of first triangle ( l == nv-2 ) always visible: 100 (0x4)
            // first edge of any other triangle always invisible
            // second edge of any triangle always visible: 010 (0x2)
            // last edge of last triangle ( l == 1 ) always visible: 001 (0x1)
            edgeVisMask = (edgeVisMask << 3) | 0x02 | (l == nv-2 ? 0x04 : 0x00) | (l == 1 ? 0x01 : 0x00);
            
            if (++edgeVisCnt >= 10)
            {
               geom->edgeVisibility->setInt(edgeVisMask, edgeVisMaskIdx++, renderFrame);
               edgeVisMask = 0;
               edgeVisCnt = 0;
            }
         }
      }
      
      j += nv;
   }
   
   // Set last edge visibility mask
   if (edgeVisCnt > 0)
   {
      geom->edgeVisibility->setInt(edgeVisMask, edgeVisMaskIdx++, renderFrame);
   }
   
   // Read positions (and interpolate/extrapolate as needed)
   if (isConst)
   {
      Alembic::Abc::P3fArraySamplePtr P = theSample.getPositions();
      
      geom->constPositions->setCount(P->size(), renderFrame);
      
      VR::VectorList vl = geom->constPositions->getVectorList(renderFrame);
      
      geom->numPoints = P->size();
      
      if (mGeoSrc->params()->verbose)
      {
         std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: " << geom->numPoints << " point(s)" << std::endl;
      }
      
      for (size_t i=0; i<P->size(); ++i)
      {
         Alembic::Abc::V3f p = P->get()[i];
         
         vl[i].set(p.x, p.y, p.z);
      }
      
      if (computeNormals)
      {
         geom->smoothNormals.push_back(computeMeshSmoothNormals(geom, (const float*) P->getData(), 0, 0.0f));
      }
   }
   else
   {
      // Reset times, frames and ntimes
      // => If deform blur is enabled, even if the mesh is not deforming, V-Ray expects a set of positions per sample
      //    as we have a time varying position parameter
      if (!mGeoSrc->params()->ignoreDeformBlur)
      {
         times = mGeoSrc->sampleTimes();
         frames = mGeoSrc->sampleFrames();
         ntimes = mGeoSrc->numTimeSamples();
      }
      
      geom->positions->clear();
      
      if (meshSamples.size() == 1 && !varyingTopology)
      {
         Alembic::Abc::P3fArraySamplePtr P = samp0->data().getPositions();
         
         geom->numPoints = P->size();
         
         if (mGeoSrc->params()->verbose)
         {
            std::cout << "[AlembicLoader] " << geom->numPoints << " point(s)" << std::endl;
         }
         
         for (size_t i=0; i<ntimes; ++i)
         {
            VR::Table<VR::Vector> &vl = geom->positions->at(frames[i]);
            vl.setCount(P->size(), true);
            
            for (size_t j=0; j<P->size(); ++j)
            {
               Alembic::Abc::V3f p = P->get()[j];
               
               vl[j].set(p.x, p.y, p.z);
            }
            
            if (computeNormals)
            {
               geom->smoothNormals.push_back(computeMeshSmoothNormals(geom, (const float*) P->getData(), 0, 0.0f));
            }
         }
      }
      else
      {
         if (varyingTopology)
         {
            //meshSamples.getSamples(renderTime, samp0, samp1, b);
            
            Alembic::Abc::P3fArraySamplePtr P = samp0->data().getPositions();
            
            geom->numPoints = P->size();
            
            if (mGeoSrc->params()->verbose)
            {
               std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: " << geom->numPoints << " point(s)" << std::endl;
            }
            
            // Get velocity
            // Note: should also allow double type values...
            const float *vel = 0;
            
            if (samp0->data().getVelocities())
            {
               vel = (const float*) samp0->data().getVelocities()->getData();
            }
            else
            {
               UserAttributes::const_iterator it = attrs.point.find("velocity");
               
               if (it == attrs.point.end() ||
                   it->second.dataCount != geom->numPoints ||
                   it->second.dataType != Float_Type ||
                   it->second.dataDim != 3)
               {
                  it = attrs.point.find("v");
                  if (it != attrs.point.end() && 
                      (it->second.dataCount != geom->numPoints ||
                       it->second.dataType != Float_Type ||
                       it->second.dataDim != 3))
                  {
                     it = attrs.point.end();
                  }
               }
               
               if (it != attrs.point.end())
               {
                  vel = (const float*) it->second.data;
               }
            }
            
            if (mGeoSrc->params()->verbose)
            {
               if (vel)
               {
                  std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: Use per-point velocity" << std::endl;
               }
            }
            
            // Get acceleration
            const float *acc = 0;
            if (vel)
            {
               UserAttributes::const_iterator it = attrs.point.find("acceleration");
               
               if (it == attrs.point.end() ||
                   it->second.dataCount != geom->numPoints ||
                   it->second.dataType != Float_Type ||
                   it->second.dataDim != 3)
               {
                  it = attrs.point.find("accel");
                  if (it == attrs.point.end() ||
                      it->second.dataCount != geom->numPoints ||
                      it->second.dataType != Float_Type ||
                      it->second.dataDim != 3)
                  {
                     it = attrs.point.find("a");
                     if (it != attrs.point.end() &&
                         (it->second.dataCount != geom->numPoints ||
                          it->second.dataType != Float_Type ||
                          it->second.dataDim != 3))
                     {
                        it = attrs.point.end();
                     }
                  }
               }
               
               if (it != attrs.point.end())
               {
                  acc = (const float*) it->second.data;
               }
            }
            
            if (mGeoSrc->params()->verbose)
            {
               if (acc)
               {
                  std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: Use per-point acceleration" << std::endl;
               }
            }
            
            if (!vel)
            {
               if (mGeoSrc->params()->verbose && !mGeoSrc->params()->ignoreDeformBlur)
               {
                  std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: No velocities found for topology varying mesh. Motion blur disabled." << std::endl;
               }
               
               for (size_t i=0; i<ntimes; ++i)
               {
                  VR::Table<VR::Vector> &vl = geom->positions->at(frames[i]);
                  vl.setCount(P->size(), true);
                  
                  for (size_t j=0; j<P->size(); ++j)
                  {
                     Alembic::Abc::V3f p = P->get()[j];
                     
                     vl[j].set(p.x, p.y, p.z);
                  }
                  
                  if (computeNormals)
                  {
                     geom->smoothNormals.push_back(computeMeshSmoothNormals(geom, (const float*) P->getData(), 0, 0.0f));
                  }
               }
            }
            else
            {
               float *Pm = (float*) malloc(3 * P->size() * sizeof(float));
               
               for (size_t i=0; i<ntimes; ++i)
               {
                  if (mGeoSrc->params()->verbose)
                  {
                     std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: Processing positions at t=" << times[i] << "..." << std::endl;
                  }
                  
                  double dt = times[i] - renderTime;
                  
                  VR::Table<VR::Vector> &vl = geom->positions->at(frames[i]);
                  vl.setCount(P->size(), true);
                  
                  float *cPm = Pm;
                  
                  if (acc)
                  {
                     const float *vvel = vel;
                     const float *vacc = acc;
                     
                     for (size_t k=0; k<P->size(); ++k, vvel+=3, vacc+=3, cPm+=3)
                     {
                        Alembic::Abc::V3f p = P->get()[k];
                        
                        cPm[0] = p.x + dt * (vvel[0] + 0.5 * dt * vacc[0]);
                        cPm[1] = p.y + dt * (vvel[1] + 0.5 * dt * vacc[1]);
                        cPm[2] = p.z + dt * (vvel[2] + 0.5 * dt * vacc[2]);
                        
                        vl[k].set(cPm[0], cPm[1], cPm[2]);
                     }
                  }
                  else
                  {
                     const float *vvel = vel;
                     
                     for (size_t k=0; k<P->size(); ++k, vvel+=3, cPm+=3)
                     {
                        Alembic::Abc::V3f p = P->get()[k];
                        
                        cPm[0] = p.x + dt * vvel[0];
                        cPm[1] = p.y + dt * vvel[1];
                        cPm[2] = p.z + dt * vvel[2];
                        
                        vl[k].set(cPm[0], cPm[1], cPm[2]);
                     }
                  }
                  
                  if (computeNormals)
                  {
                     geom->smoothNormals.push_back(computeMeshSmoothNormals(geom, Pm, 0, 0.0f));
                  }
               }
               
               free(Pm);
            }
         }
         else
         {
            geom->numPoints = 0;
            
            for (size_t i=0; i<ntimes; ++i)
            {
               if (mGeoSrc->params()->verbose)
               {
                  std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: Processing positions at t=" << times[i] << "..." << std::endl;
               }
               
               t = times[i];
               
               meshSamples.getSamples(t, samp0, samp1, b);
               
               Alembic::Abc::P3fArraySamplePtr P0 = samp0->data().getPositions();
               
               if (geom->numPoints > 0 && P0->size() != geom->numPoints)
               {
                  std::cerr << "[AlembicLoader] UpdateGeometry::readBaseMesh: Changing position count amongst samples" << std::endl;
                  rv = false;
                  break;
               }
               
               if (mGeoSrc->params()->verbose && geom->numPoints <= 0)
               {
                  std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: " << P0->size() << " point(s)" << std::endl;
               }
               
               geom->numPoints = P0->size();
               
               VR::Table<VR::Vector> &vl = geom->positions->at(frames[i]);
               vl.setCount(geom->numPoints, true);
               
               if (b > 0.0)
               {
                  if (mGeoSrc->params()->verbose)
                  {
                     std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: Interpolate positions at t0=" << samp0->time() << ", t1=" << samp1->time() << ", blend=" << b << std::endl;
                  }
                  
                  Alembic::Abc::P3fArraySamplePtr P1 = samp1->data().getPositions();
                  
                  double a = 1.0 - b;
                  
                  for (size_t k=0; k<P0->size(); ++k)
                  {
                     Alembic::Abc::V3f p0 = P0->get()[k];
                     Alembic::Abc::V3f p1 = P1->get()[k];
                     
                     vl[k].set(a * p0.x + b * p1.x,
                               a * p0.y + b * p1.y,
                               a * p0.z + b * p1.z);
                  }
                  
                  if (computeNormals)
                  {
                     geom->smoothNormals.push_back(computeMeshSmoothNormals(geom, (const float*) P0->getData(), (const float*) P1->getData(), b));
                  }
               }
               else
               {
                  if (mGeoSrc->params()->verbose)
                  {
                     std::cout << "[AlembicLoader] UpdateGeometry::readBaseMesh: Read positions at t=" << samp0->time() << std::endl;
                  }
                  
                  for (size_t k=0; k<P0->size(); ++k)
                  {
                     Alembic::Abc::V3f p = P0->get()[k];
                     
                     vl[k].set(p.x, p.y, p.z);
                  }
                  
                  if (computeNormals)
                  {
                     geom->smoothNormals.push_back(computeMeshSmoothNormals(geom, (const float*) P0->getData(), 0, 0.0f));
                  }
               }
            }
         }
      }
   }
   
   return rv;
}

template <class MeshSchema>
void UpdateGeometry::setMeshSmoothNormals(AlembicNodeT<Alembic::Abc::ISchemaObject<MeshSchema> > &node,
                                          AbcVRayGeom *geom)
{
   double renderFrame = mGeoSrc->renderFrame();
   
   if (geom->constNormals)
   {
      if (geom->smoothNormals.size() == 1)
      {
         geom->constNormals->setCount(geom->numPoints, renderFrame);
         VR::VectorList nl = geom->constNormals->getVectorList(renderFrame);
         
         geom->constFaceNormals->setCount(3 * geom->numTriangles, renderFrame);
         VR::IntList il = geom->constFaceNormals->getIntList(renderFrame);
         
         const float *N = geom->smoothNormals[0];
         
         for (unsigned int i=0, j=0; i<geom->numPoints; ++i, j+=3)
         {
            nl[i].set(N[j], N[j+1], N[j+2]);
         }
         
         for (unsigned int i=0, k=0; i<geom->numTriangles; ++i)
         {
            for (unsigned int j=0; j<3; ++j, ++k)
            {
               il[k] = geom->toPointIndex[k];
            }
         }
      }
      else
      {
         std::cout << "[AlembicLoader] Computed smooth normal samples and schema samples do no match, ignoring them" << std::endl;
         
         geom->constNormals->setCount(0, renderFrame);
         geom->constFaceNormals->setCount(0, renderFrame);
      }
   }
   else
   {
      geom->normals->clear();
      geom->faceNormals->clear();
      
      MeshSchema &schema = node.typedObject().getSchema();
      
      bool varyingTopology = schema.getTopologyVariance() == Alembic::AbcGeom::kHeterogenousTopology;
      bool singleSample = (mGeoSrc->params()->ignoreDeformBlur || varyingTopology);
      double renderTime = mGeoSrc->renderTime();
      double *times = (singleSample ? &renderTime : mGeoSrc->sampleTimes());
      double *frames = (singleSample ? &renderFrame : mGeoSrc->sampleFrames());
      size_t ntimes = (singleSample ? 1 : mGeoSrc->numTimeSamples());
      
      if (geom->smoothNormals.size() == ntimes)
      {
         for (size_t i=0; i<ntimes; ++i)
         {
            const float *N = geom->smoothNormals[i];
            
            VR::Table<VR::Vector> &nl = geom->normals->at(frames[i]);
            nl.setCount(geom->numPoints, true);
            
            VR::Table<int> &il = geom->faceNormals->at(frames[i]);
            il.setCount(3 * geom->numTriangles, true);
            
            for (unsigned int j=0, k=0; j<geom->numPoints; ++j, k+=3)
            {
               nl[j].set(N[k], N[k+1], N[k+2]);
            }
            
            for (unsigned int j=0, l=0; j<geom->numTriangles; ++j)
            {
               for (unsigned int k=0; k<3; ++k, ++l)
               {
                  il[l] = geom->toPointIndex[l];
               }
            }
         }
      }
      else
      {
         std::cout << "[AlembicLoader] Computed smooth normal samples and schema samples do no match, ignoring them" << std::endl;
      }
   }
}


template <class MeshSchema>
size_t UpdateGeometry::readMeshUVs(AlembicNodeT<Alembic::Abc::ISchemaObject<MeshSchema> > &node,
                                   AbcVRayGeom *geom,
                                   const UserAttrsSet &attrs,
                                   bool primaryOnly)
{
   size_t numUVSets = 0;
   
   MeshSchema &schema = node.typedObject().getSchema();
   
   double renderTime = mGeoSrc->renderTime();
   double renderFrame = mGeoSrc->renderFrame();
   
   std::set<std::string> uvNames;
   UserAttribute uv;
   
   Alembic::AbcGeom::IV2fGeomParam uvParam = schema.getUVsParam();
   
   if (uvParam.valid())
   {
      char tmp[128];
      int isuffix = 1;
      
      std::string name = Alembic::AbcGeom::GetSourceName(uvParam.getMetaData());
      
      if (name.length() == 0)
      {
         name = "map1";
      }
      
      // Make sure we have a 'valid' name for master uv set (i.e. non conflicting with any other user attribute)
      while (uvNames.find(name) != uvNames.end() ||
             attrs.object.find(name) != attrs.object.end() ||
             attrs.primitive.find(name) != attrs.primitive.end() ||
             attrs.point.find(name) != attrs.point.end() ||
             attrs.vertex.find(name) != attrs.vertex.end())
      {
         ++isuffix;
         sprintf(tmp, "map%d", isuffix);
         name = tmp;
      }
      
      InitUserAttribute(uv);
      
      if (ReadUserAttribute(uv, uvParam.getParent(), uvParam.getHeader(), renderTime, true, false, mGeoSrc->params()->verbose))
      {
         if (SetUserAttribute(geom, name.c_str(), uv, renderTime, uvNames, mGeoSrc->params()->verbose))
         {
            int count = geom->channelNames->getCount(renderFrame);
            geom->channelNames->setCount(count + 1, renderFrame);
            geom->channelNames->setString(name.c_str(), count, renderFrame);
            
            if (mGeoSrc->params()->verbose)
            {
               std::cout << "[AlembicLoader] UpdateGeometry::readMeshUVs: Added UV set \"" << name << "\"" << std::endl;
            }
            
            ++numUVSets;
         }
      }
      
      DestroyUserAttribute(uv);
   }
   
   if (!primaryOnly)
   {
      std::map<std::string, Alembic::AbcGeom::IV2fGeomParam>::const_iterator uvit = attrs.uvs.begin();
      
      while (uvit != attrs.uvs.end())
      {
         if (uvNames.find(uvit->first) == uvNames.end())
         {
            InitUserAttribute(uv);
         
            if (ReadUserAttribute(uv, uvit->second.getParent(), uvit->second.getHeader(), renderTime, true, false, mGeoSrc->params()->verbose))
            {
               if (SetUserAttribute(geom, uvit->first.c_str(), uv, renderTime, uvNames, mGeoSrc->params()->verbose))
               {
                  int count = geom->channelNames->getCount(renderFrame);
                  geom->channelNames->setCount(count + 1, renderFrame);
                  geom->channelNames->setString(uvit->first.c_str(), count, renderFrame);
                  
                  if (mGeoSrc->params()->verbose)
                  {
                     std::cout << "[AlembicLoader] UpdateGeometry::readMeshUVs: Added UV set \"" << uvit->first << "\"" << std::endl;
                  }
                  
                  ++numUVSets;
               }
            }
         
            DestroyUserAttribute(uv);
         }
      
         ++uvit;
      }
   }
   
   return numUVSets;
}


#endif
