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
    AMbyteSpan bs;
    FuncCallContext *funcctx;

    LOGF();

    if (SRF_IS_FIRSTCALL()) {
        MemoryContext oldctx;
        funcctx = SRF_FIRSTCALL_INIT();
        oldctx = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
        doc = AUTODOC_GETARG(0);
		state = (autodoc_ChangesState*)palloc(sizeof(autodoc_ChangesState));

        changes = AMstackItems(&doc->stack,
                               AMgetChanges(doc->doc, NULL),
                               _abort_cb,
                               AMexpect(AM_VAL_TYPE_CHANGE));

        funcctx->max_calls = AMitemsSize(&changes);
        state->changes = palloc(sizeof(autochange_Autochange*) * funcctx->max_calls);

        for (int i = 0; (item = AMitemsNext(&changes, 1)) != NULL; i++) {
            AMitemToChange(item, &itemchange);
            bs = AMchangeRawBytes(itemchange);
            state->changes[i] = new_expanded_autochange(CurrentMemoryContext, bs.src, bs.count);
        }

        funcctx->user_fctx = state;
        MemoryContextSwitchTo(oldctx);
    }
    funcctx = SRF_PERCALL_SETUP();

    state = (autodoc_ChangesState*) funcctx->user_fctx;

	if (funcctx->call_cntr >= funcctx->max_calls) {
		SRF_RETURN_DONE(funcctx);
	}
	else {
        change = state->changes[funcctx->call_cntr];
        SRF_RETURN_NEXT(funcctx, EOHPGetRWDatum(&change->hdr));
    }
}
