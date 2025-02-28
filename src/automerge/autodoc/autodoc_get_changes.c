#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_get_changes);
Datum autodoc_get_changes(PG_FUNCTION_ARGS)
{
    autodoc_Autodoc *doc;
    autochange_Autochange *change;
    autodoc_ChangesState *state;
    AMitem* item = NULL;
    AMitems changes;
    AMchange* itemchange;
    AMbyteSpan rawbs;
    uint8_t *raw = NULL;
    FuncCallContext *funcctx;

    LOGF();

    if (SRF_IS_FIRSTCALL()) {
        MemoryContext oldctx;
        funcctx = SRF_FIRSTCALL_INIT();
        oldctx = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
        doc = AUTODOC_GETARG(0);
		state = (autodoc_ChangesState*)palloc(sizeof(autodoc_ChangesState));

        state->doc = doc;
        changes = AMstackItems(&doc->stack,
                               AMgetChanges(doc->doc, NULL),
                               abort_cb,
                               AMexpect(AM_VAL_TYPE_CHANGE));

        state->changes = (AMitems *) palloc(sizeof(AMitems));
        memcpy(state->changes, &changes, sizeof(AMitems));
        funcctx->user_fctx = state;
        funcctx->max_calls = AMitemsSize(state->changes);
        MemoryContextSwitchTo(oldctx);
    }
    funcctx = SRF_PERCALL_SETUP();

    state = (autodoc_ChangesState*) funcctx->user_fctx;

    if ((item = AMitemsNext(state->changes, 1)) != NULL) {
        AMitemToChange(item, &itemchange);
        rawbs = AMchangeRawBytes(itemchange);
        raw = palloc0(rawbs.count);
        memcpy(raw, rawbs.src, rawbs.count);
        change = new_expanded_autochange(CurrentMemoryContext, raw, rawbs.count);
        SRF_RETURN_NEXT(funcctx, EOHPGetRWDatum(&change->hdr));
    } else {
        SRF_RETURN_DONE(funcctx);
    }
}
