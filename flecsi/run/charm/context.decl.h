#ifndef _DECL_context_H_
#define _DECL_context_H_
#include "charm++.h"
#include "envelope.h"
#include <memory>
#include "sdag.h"
namespace flecsi {
namespace run {
namespace charm {
/* DECLS: group ContextGroup: IrrGroup{
ContextGroup();
void top_level_task();
};
 */
 class ContextGroup;
 class CkIndex_ContextGroup;
 class CProxy_ContextGroup;
 class CProxyElement_ContextGroup;
 class CProxySection_ContextGroup;
/* --------------- index object ------------------ */
class CkIndex_ContextGroup:public CkIndex_IrrGroup{
  public:
    typedef ContextGroup local_t;
    typedef CkIndex_ContextGroup index_t;
    typedef CProxy_ContextGroup proxy_t;
    typedef CProxyElement_ContextGroup element_t;
    typedef CProxySection_ContextGroup section_t;

    static int __idx;
    static void __register(const char *s, size_t size);
    /* DECLS: ContextGroup();
     */
    // Entry point registration at startup
    
    static int reg_ContextGroup_void();
    // Entry point index lookup
    
    inline static int idx_ContextGroup_void() {
      static int epidx = reg_ContextGroup_void();
      return epidx;
    }

    
    static int ckNew() { return idx_ContextGroup_void(); }
    
    static void _call_ContextGroup_void(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_ContextGroup_void(void* impl_msg, void* impl_obj);
    /* DECLS: void top_level_task();
     */
    // Entry point registration at startup
    
    static int reg_top_level_task_void();
    // Entry point index lookup
    
    inline static int idx_top_level_task_void() {
      static int epidx = reg_top_level_task_void();
      return epidx;
    }

    
    inline static int idx_top_level_task(void (ContextGroup::*)() ) {
      return idx_top_level_task_void();
    }


    
    static int top_level_task() { return idx_top_level_task_void(); }
    
    static void _call_top_level_task_void(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_top_level_task_void(void* impl_msg, void* impl_obj);
};
/* --------------- element proxy ------------------ */
class CProxyElement_ContextGroup: public CProxyElement_IrrGroup{
  public:
    typedef ContextGroup local_t;
    typedef CkIndex_ContextGroup index_t;
    typedef CProxy_ContextGroup proxy_t;
    typedef CProxyElement_ContextGroup element_t;
    typedef CProxySection_ContextGroup section_t;


    /* TRAM aggregators */

    CProxyElement_ContextGroup(void) {
    }
    CProxyElement_ContextGroup(const IrrGroup *g) : CProxyElement_IrrGroup(g){
    }
    CProxyElement_ContextGroup(CkGroupID _gid,int _onPE,CK_DELCTOR_PARAM) : CProxyElement_IrrGroup(_gid,_onPE,CK_DELCTOR_ARGS){
    }
    CProxyElement_ContextGroup(CkGroupID _gid,int _onPE) : CProxyElement_IrrGroup(_gid,_onPE){
    }

    int ckIsDelegated(void) const
    { return CProxyElement_IrrGroup::ckIsDelegated(); }
    inline CkDelegateMgr *ckDelegatedTo(void) const
    { return CProxyElement_IrrGroup::ckDelegatedTo(); }
    inline CkDelegateData *ckDelegatedPtr(void) const
    { return CProxyElement_IrrGroup::ckDelegatedPtr(); }
    CkGroupID ckDelegatedIdx(void) const
    { return CProxyElement_IrrGroup::ckDelegatedIdx(); }
inline void ckCheck(void) const {CProxyElement_IrrGroup::ckCheck();}
CkChareID ckGetChareID(void) const
   {return CProxyElement_IrrGroup::ckGetChareID();}
CkGroupID ckGetGroupID(void) const
   {return CProxyElement_IrrGroup::ckGetGroupID();}
operator CkGroupID () const { return ckGetGroupID(); }

    inline void setReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxyElement_IrrGroup::setReductionClient(fn,param); }
    inline void ckSetReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxyElement_IrrGroup::ckSetReductionClient(fn,param); }
    inline void ckSetReductionClient(CkCallback *cb) const
    { CProxyElement_IrrGroup::ckSetReductionClient(cb); }
int ckGetGroupPe(void) const
{return CProxyElement_IrrGroup::ckGetGroupPe();}

    void ckDelegate(CkDelegateMgr *dTo,CkDelegateData *dPtr=NULL)
    {       CProxyElement_IrrGroup::ckDelegate(dTo,dPtr); }
    void ckUndelegate(void)
    {       CProxyElement_IrrGroup::ckUndelegate(); }
    void pup(PUP::er &p)
    {       CProxyElement_IrrGroup::pup(p);
    }
    void ckSetGroupID(CkGroupID g) {
      CProxyElement_IrrGroup::ckSetGroupID(g);
    }
    ContextGroup* ckLocalBranch(void) const {
      return ckLocalBranch(ckGetGroupID());
    }
    static ContextGroup* ckLocalBranch(CkGroupID gID) {
      return (ContextGroup*)CkLocalBranch(gID);
    }
/* DECLS: ContextGroup();
 */
    

/* DECLS: void top_level_task();
 */
    
    void top_level_task(const CkEntryOptions *impl_e_opts=NULL);

};
/* ---------------- collective proxy -------------- */
class CProxy_ContextGroup: public CProxy_IrrGroup{
  public:
    typedef ContextGroup local_t;
    typedef CkIndex_ContextGroup index_t;
    typedef CProxy_ContextGroup proxy_t;
    typedef CProxyElement_ContextGroup element_t;
    typedef CProxySection_ContextGroup section_t;

    CProxy_ContextGroup(void) {
    }
    CProxy_ContextGroup(const IrrGroup *g) : CProxy_IrrGroup(g){
    }
    CProxy_ContextGroup(CkGroupID _gid,CK_DELCTOR_PARAM) : CProxy_IrrGroup(_gid,CK_DELCTOR_ARGS){  }
    CProxy_ContextGroup(CkGroupID _gid) : CProxy_IrrGroup(_gid){  }
    CProxyElement_ContextGroup operator[](int onPE) const
      {return CProxyElement_ContextGroup(ckGetGroupID(),onPE,CK_DELCTOR_CALL);}

    int ckIsDelegated(void) const
    { return CProxy_IrrGroup::ckIsDelegated(); }
    inline CkDelegateMgr *ckDelegatedTo(void) const
    { return CProxy_IrrGroup::ckDelegatedTo(); }
    inline CkDelegateData *ckDelegatedPtr(void) const
    { return CProxy_IrrGroup::ckDelegatedPtr(); }
    CkGroupID ckDelegatedIdx(void) const
    { return CProxy_IrrGroup::ckDelegatedIdx(); }
inline void ckCheck(void) const {CProxy_IrrGroup::ckCheck();}
CkChareID ckGetChareID(void) const
   {return CProxy_IrrGroup::ckGetChareID();}
CkGroupID ckGetGroupID(void) const
   {return CProxy_IrrGroup::ckGetGroupID();}
operator CkGroupID () const { return ckGetGroupID(); }

    inline void setReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxy_IrrGroup::setReductionClient(fn,param); }
    inline void ckSetReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxy_IrrGroup::ckSetReductionClient(fn,param); }
    inline void ckSetReductionClient(CkCallback *cb) const
    { CProxy_IrrGroup::ckSetReductionClient(cb); }

    void ckDelegate(CkDelegateMgr *dTo,CkDelegateData *dPtr=NULL)
    {       CProxy_IrrGroup::ckDelegate(dTo,dPtr); }
    void ckUndelegate(void)
    {       CProxy_IrrGroup::ckUndelegate(); }
    void pup(PUP::er &p)
    {       CProxy_IrrGroup::pup(p);
    }
    void ckSetGroupID(CkGroupID g) {
      CProxy_IrrGroup::ckSetGroupID(g);
    }
    ContextGroup* ckLocalBranch(void) const {
      return ckLocalBranch(ckGetGroupID());
    }
    static ContextGroup* ckLocalBranch(CkGroupID gID) {
      return (ContextGroup*)CkLocalBranch(gID);
    }
/* DECLS: ContextGroup();
 */
    
    static CkGroupID ckNew(const CkEntryOptions *impl_e_opts=NULL);

/* DECLS: void top_level_task();
 */
    
    void top_level_task(const CkEntryOptions *impl_e_opts=NULL);
    
    void top_level_task(int npes, int *pes, const CkEntryOptions *impl_e_opts=NULL);
    
    void top_level_task(CmiGroup &grp, const CkEntryOptions *impl_e_opts=NULL);

};
/* ---------------- section proxy -------------- */
class CProxySection_ContextGroup: public CProxySection_IrrGroup{
  public:
    typedef ContextGroup local_t;
    typedef CkIndex_ContextGroup index_t;
    typedef CProxy_ContextGroup proxy_t;
    typedef CProxyElement_ContextGroup element_t;
    typedef CProxySection_ContextGroup section_t;

    CProxySection_ContextGroup(void) {
    }
    CProxySection_ContextGroup(const IrrGroup *g) : CProxySection_IrrGroup(g){
    }
    CProxySection_ContextGroup(const CkGroupID &_gid,const int *_pelist,int _npes, CK_DELCTOR_PARAM) : CProxySection_IrrGroup(_gid,_pelist,_npes,CK_DELCTOR_ARGS){  }
    CProxySection_ContextGroup(const CkGroupID &_gid,const int *_pelist,int _npes, int factor = USE_DEFAULT_BRANCH_FACTOR) : CProxySection_IrrGroup(_gid,_pelist,_npes,factor){  }
    CProxySection_ContextGroup(int n,const CkGroupID *_gid, int const * const *_pelist,const int *_npes, int factor = USE_DEFAULT_BRANCH_FACTOR) : CProxySection_IrrGroup(n,_gid,_pelist,_npes,factor){  }
    CProxySection_ContextGroup(int n,const CkGroupID *_gid, int const * const *_pelist,const int *_npes, CK_DELCTOR_PARAM) : CProxySection_IrrGroup(n,_gid,_pelist,_npes,CK_DELCTOR_ARGS){  }

    int ckIsDelegated(void) const
    { return CProxySection_IrrGroup::ckIsDelegated(); }
    inline CkDelegateMgr *ckDelegatedTo(void) const
    { return CProxySection_IrrGroup::ckDelegatedTo(); }
    inline CkDelegateData *ckDelegatedPtr(void) const
    { return CProxySection_IrrGroup::ckDelegatedPtr(); }
    CkGroupID ckDelegatedIdx(void) const
    { return CProxySection_IrrGroup::ckDelegatedIdx(); }
inline void ckCheck(void) const {CProxySection_IrrGroup::ckCheck();}
CkChareID ckGetChareID(void) const
   {return CProxySection_IrrGroup::ckGetChareID();}
CkGroupID ckGetGroupID(void) const
   {return CProxySection_IrrGroup::ckGetGroupID();}
operator CkGroupID () const { return ckGetGroupID(); }

    inline void setReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxySection_IrrGroup::setReductionClient(fn,param); }
    inline void ckSetReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxySection_IrrGroup::ckSetReductionClient(fn,param); }
    inline void ckSetReductionClient(CkCallback *cb) const
    { CProxySection_IrrGroup::ckSetReductionClient(cb); }
inline int ckGetNumSections() const
{ return CProxySection_IrrGroup::ckGetNumSections(); }
inline CkSectionInfo &ckGetSectionInfo()
{ return CProxySection_IrrGroup::ckGetSectionInfo(); }
inline CkSectionID *ckGetSectionIDs()
{ return CProxySection_IrrGroup::ckGetSectionIDs(); }
inline CkSectionID &ckGetSectionID()
{ return CProxySection_IrrGroup::ckGetSectionID(); }
inline CkSectionID &ckGetSectionID(int i)
{ return CProxySection_IrrGroup::ckGetSectionID(i); }
inline CkGroupID ckGetGroupIDn(int i) const
{ return CProxySection_IrrGroup::ckGetGroupIDn(i); }
inline const int *ckGetElements() const
{ return CProxySection_IrrGroup::ckGetElements(); }
inline const int *ckGetElements(int i) const
{ return CProxySection_IrrGroup::ckGetElements(i); }
inline int ckGetNumElements() const
{ return CProxySection_IrrGroup::ckGetNumElements(); } 
inline int ckGetNumElements(int i) const
{ return CProxySection_IrrGroup::ckGetNumElements(i); }

    void ckDelegate(CkDelegateMgr *dTo,CkDelegateData *dPtr=NULL)
    {       CProxySection_IrrGroup::ckDelegate(dTo,dPtr); }
    void ckUndelegate(void)
    {       CProxySection_IrrGroup::ckUndelegate(); }
    void pup(PUP::er &p)
    {       CProxySection_IrrGroup::pup(p);
    }
    void ckSetGroupID(CkGroupID g) {
      CProxySection_IrrGroup::ckSetGroupID(g);
    }
    ContextGroup* ckLocalBranch(void) const {
      return ckLocalBranch(ckGetGroupID());
    }
    static ContextGroup* ckLocalBranch(CkGroupID gID) {
      return (ContextGroup*)CkLocalBranch(gID);
    }
/* DECLS: ContextGroup();
 */
    

/* DECLS: void top_level_task();
 */
    
    void top_level_task(const CkEntryOptions *impl_e_opts=NULL);

};
#define ContextGroup_SDAG_CODE 
typedef CBaseT1<Group, CProxy_ContextGroup>CBase_ContextGroup;

} // namespace charm

} // namespace run

} // namespace flecsi

namespace flecsi {
namespace run {
namespace charm {
/* ---------------- method closures -------------- */
class Closure_ContextGroup {
  public:


    struct top_level_task_2_closure;

};

} // namespace charm

} // namespace run

} // namespace flecsi

extern void _registercontext(void);
extern "C" void CkRegisterMainModule(void);
#endif
