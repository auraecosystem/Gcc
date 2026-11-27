2026-11-26  Eric Botcazou  <ebotcazou@adacore.com>

	PR ada/128723
	PR ada/129058
	* sem_aggr.adb (Resolve_Array_Aggregate.Resolve_Aggr_Expr):
	Copy the expression by means of New_Copy_Tree.
	(Resolve_Delta_Array_Aggregate): Likewise.

2026-11-23  Eric Botcazou  <ebotcazou@adacore.com>

	* exp_ch4.adb (Expand_Allocator_Expression): Allocate the maximum
