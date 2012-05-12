#ifndef _BUILDMODEL_H
#define _BUILDMODEL_H

int AllocModel (RenderModel::CModel *modelP);
int FreeModelItems (RenderModel::CModel *modelP);
void FreeAllPolyModelItems (void);
void SortFaces (RenderModel::CSubModel *subModelP, int left, int right, CBitmap *pTextures);
void SortFaceVerts (RenderModel::CModel *modelP, RenderModel::CSubModel *psm, RenderModel::CVertex *psv);
void SetupModel (RenderModel::CModel *modelP, int bHires, int bSort);
int BuildModel (CObject *objP, int nModel, CPolyModel *polyModelP, CDynamicArray<CBitmap*>& modelBitmaps, CFloatVector* objColorP, int bHires);

int BuildModelFromASE (CObject *objP, int nModel);
int BuildModelFromPOF (CObject *objP, int nModel, CPolyModel *polyModelP, CDynamicArray<CBitmap*>& modelBitmaps, CFloatVector *objColorP);

//------------------------------------------------------------------------------

#endif //_BUILDMODEL_H
//eof
