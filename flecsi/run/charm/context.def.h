namespace flecsi {
namespace run {
namespace charm {
/* ---------------- method closures -------------- */
#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY

    struct Closure_ContextGroup::top_level_task_2_closure : public SDAG::Closure {
      

      top_level_task_2_closure() {
        init();
      }
      top_level_task_2_closure(CkMigrateMessage*) {
        init();
      }
            void pup(PUP::er& __p) {
        packClosure(__p);
      }
      virtual ~top_level_task_2_closure() {
      }
      PUPable_decl(SINGLE_ARG(top_level_task_2_closure));
    };
#endif /* CK_TEMPLATES_ONLY */


} // namespace charm

} // namespace run

} // namespace flecsi

namespace flecsi {
namespace run {
namespace charm {
/* DEFS: group ContextGroup: IrrGroup{
ContextGroup();
void top_level_task();
};
 */
#ifndef CK_TEMPLATES_ONLY
 int CkIndex_ContextGroup::__idx=0;
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
/* DEFS: ContextGroup();
 */
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void top_level_task();
 */
void CProxyElement_ContextGroup::top_level_task(const CkEntryOptions *impl_e_opts)
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  if (ckIsDelegated()) {
     CkGroupMsgPrep(CkIndex_ContextGroup::idx_top_level_task_void(), impl_msg, ckGetGroupID());
     ckDelegatedTo()->GroupSend(ckDelegatedPtr(),CkIndex_ContextGroup::idx_top_level_task_void(), impl_msg, ckGetGroupPe(), ckGetGroupID());
  } else {
    CkSendMsgBranch(CkIndex_ContextGroup::idx_top_level_task_void(), impl_msg, ckGetGroupPe(), ckGetGroupID(),0);
  }
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: ContextGroup();
 */
CkGroupID CProxy_ContextGroup::ckNew(const CkEntryOptions *impl_e_opts)
{
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  UsrToEnv(impl_msg)->setMsgtype(BocInitMsg);
  CkGroupID gId = CkCreateGroup(CkIndex_ContextGroup::__idx, CkIndex_ContextGroup::idx_ContextGroup_void(), impl_msg);
  return gId;
}

// Entry point registration function
int CkIndex_ContextGroup::reg_ContextGroup_void() {
  int epidx = CkRegisterEp("ContextGroup()",
      reinterpret_cast<CkCallFnPtr>(_call_ContextGroup_void), 0, __idx, 0);
  return epidx;
}

void CkIndex_ContextGroup::_call_ContextGroup_void(void* impl_msg, void* impl_obj_void)
{
  ContextGroup* impl_obj = static_cast<ContextGroup*>(impl_obj_void);
  new (impl_obj_void) ContextGroup();
  if(UsrToEnv(impl_msg)->isVarSysMsg() == 0)
    CkFreeSysMsg(impl_msg);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void top_level_task();
 */
void CProxy_ContextGroup::top_level_task(const CkEntryOptions *impl_e_opts)
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  if (ckIsDelegated()) {
     CkGroupMsgPrep(CkIndex_ContextGroup::idx_top_level_task_void(), impl_msg, ckGetGroupID());
     ckDelegatedTo()->GroupBroadcast(ckDelegatedPtr(),CkIndex_ContextGroup::idx_top_level_task_void(), impl_msg, ckGetGroupID());
  } else CkBroadcastMsgBranch(CkIndex_ContextGroup::idx_top_level_task_void(), impl_msg, ckGetGroupID(),0);
}
void CProxy_ContextGroup::top_level_task(int npes, int *pes, const CkEntryOptions *impl_e_opts) {
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  CkSendMsgBranchMulti(CkIndex_ContextGroup::idx_top_level_task_void(), impl_msg, ckGetGroupID(), npes, pes,0);
}
void CProxy_ContextGroup::top_level_task(CmiGroup &grp, const CkEntryOptions *impl_e_opts) {
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  CkSendMsgBranchGroup(CkIndex_ContextGroup::idx_top_level_task_void(), impl_msg, ckGetGroupID(), grp,0);
}

// Entry point registration function
int CkIndex_ContextGroup::reg_top_level_task_void() {
  int epidx = CkRegisterEp("top_level_task()",
      reinterpret_cast<CkCallFnPtr>(_call_top_level_task_void), 0, __idx, 0);
  return epidx;
}

void CkIndex_ContextGroup::_call_top_level_task_void(void* impl_msg, void* impl_obj_void)
{
  ContextGroup* impl_obj = static_cast<ContextGroup*>(impl_obj_void);
  impl_obj->top_level_task();
  if(UsrToEnv(impl_msg)->isVarSysMsg() == 0)
    CkFreeSysMsg(impl_msg);
}
PUPable_def(SINGLE_ARG(Closure_ContextGroup::top_level_task_2_closure))
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: ContextGroup();
 */
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void top_level_task();
 */
void CProxySection_ContextGroup::top_level_task(const CkEntryOptions *impl_e_opts)
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  if (ckIsDelegated()) {
     ckDelegatedTo()->GroupSectionSend(ckDelegatedPtr(),CkIndex_ContextGroup::idx_top_level_task_void(), impl_msg, ckGetNumSections(), ckGetSectionIDs());
  } else {
    void *impl_msg_tmp;
    for (int i=0; i<ckGetNumSections(); ++i) {
       impl_msg_tmp= (i<ckGetNumSections()-1) ? CkCopyMsg((void **) &impl_msg):impl_msg;
       CkSendMsgBranchMulti(CkIndex_ContextGroup::idx_top_level_task_void(), impl_msg_tmp, ckGetGroupIDn(i), ckGetNumElements(i), ckGetElements(i),0);
    }
  }
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void CkIndex_ContextGroup::__register(const char *s, size_t size) {
  __idx = CkRegisterChare(s, size, TypeGroup);
  CkRegisterBase(__idx, CkIndex_IrrGroup::__idx);
   CkRegisterGroupIrr(__idx,ContextGroup::isIrreducible());
  // REG: ContextGroup();
  idx_ContextGroup_void();
  CkRegisterDefaultCtor(__idx, idx_ContextGroup_void());

  // REG: void top_level_task();
  idx_top_level_task_void();

}
#endif /* CK_TEMPLATES_ONLY */

} // namespace charm

} // namespace run

} // namespace flecsi

#ifndef CK_TEMPLATES_ONLY
void _registercontext(void)
{
  static int _done = 0; if(_done) return; _done = 1;
using namespace flecsi;
using namespace run;
using namespace charm;
/* REG: group ContextGroup: IrrGroup{
ContextGroup();
void top_level_task();
};
*/
  CkIndex_ContextGroup::__register("ContextGroup", sizeof(ContextGroup));




}
extern "C" void CkRegisterMainModule(void) {
  _registercontext();
}
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
template <>
void flecsi::run::charm::CBase_ContextGroup::virtual_pup(PUP::er &p) {
    recursive_pup<flecsi::run::charm::ContextGroup>(dynamic_cast<flecsi::run::charm::ContextGroup*>(this), p);
}
#endif /* CK_TEMPLATES_ONLY */
