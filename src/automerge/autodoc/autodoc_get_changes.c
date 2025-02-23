#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_get_changes);
Datum autodoc_get_changes(PG_FUNCTION_ARGS)
{
    autodoc_Autodoc *doc;
    autochange_Autochange *change;
    AMitems changes;
    AMitems *changesp;
    AMitem* item = NULL;
    AMchange* itemchange;
    AMbyteSpan bs;
    FuncCallContext *funcctx;

    doc = AUTODOC_GETARG(0);

    if (SRF_IS_FIRSTCALL()) {
        MemoryContext oldctx;
        funcctx = SRF_FIRSTCALL_INIT();
        oldctx = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        changes = AMstackItems(&doc->stack,
                               AMgetChanges(doc->doc, NULL),
                               abort_cb,
                               AMexpect(AM_VAL_TYPE_CHANGE));

        funcctx->user_fctx = &changes;
        funcctx->max_calls = AMitemsSize(&changes);
        MemoryContextSwitchTo(oldctx);
    }
    funcctx = SRF_PERCALL_SETUP();

    changesp = (AMitems*) funcctx->user_fctx;
    if ((item = AMitemsNext(changesp, 1)) != NULL && funcctx->call_cntr < funcctx->max_calls) {
        AMitemToChange(item, &itemchange);
        change = new_expanded_autochange(NULL, CurrentMemoryContext);
        bs = AMchangeRawBytes(itemchange);
        AMitemToChange(AMstackItem(&change->stack,
                                   AMchangeFromBytes(bs.src, bs.count),
                                   abort_cb,
                                   AMexpect(AM_VAL_TYPE_CHANGE)),
                       &change->change);
    SRF_RETURN_NEXT(funcctx, EOHPGetRWDatum(&change->hdr));
    } else {
        SRF_RETURN_DONE(funcctx);
    }
}
