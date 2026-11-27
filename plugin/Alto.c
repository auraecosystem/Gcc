struct plugin_file_info
  void *handle;
  struct plugin_symtab symtab;
  struct plugin_symtab conflicts;
  bool skip_file;
};

/* List item with name of the file with offloading.  */
@@ -159,7 +160,7 @@ enum symbol_style
static char *arguments_file_name;
static ld_plugin_register_claim_file register_claim_file;
static ld_plugin_register_all_symbols_read register_all_symbols_read;
static ld_plugin_get_symbols get_symbols, get_symbols_v2;
static ld_plugin_get_symbols get_symbols, get_symbols_v2, get_symbols_v3;
static ld_plugin_register_cleanup register_cleanup;
static ld_plugin_add_input_file add_input_file;
static ld_plugin_add_input_library add_input_library;
@@ -547,29 +548,52 @@ free_symtab (struct plugin_symtab *symtab)
static void
write_resolution (void)
{
  unsigned int i;
  unsigned int i, included_files = 0;
  FILE *f;

  check (resolution_file, LDPL_FATAL, "resolution file not specified");
  f = fopen (resolution_file, "w");
  check (f, LDPL_FATAL, "could not open file");

  fprintf (f, "%d\n", num_claimed_files);

  for (i = 0; i < num_claimed_files; i++)
    {
      struct plugin_file_info *info = &claimed_files[i];
      struct plugin_symtab *symtab = &info->symtab;
      struct ld_plugin_symbol *syms = symtab->syms;

      /* Version 2 of API supports IRONLY_EXP resolution that is
         accepted by GCC-4.7 and newer.  */
      if (get_symbols_v2)
	 accepted by GCC-4.7 and newer.
	 Version 3 can return LDPS_NO_SYMS that means the object
	 will not be used at all.  */
      if (get_symbols_v3)
	{
	  enum ld_plugin_status status
	    = get_symbols_v3 (info->handle, symtab->nsyms, syms);
	  if (status == LDPS_NO_SYMS)
	    {
	      info->skip_file = true;
	      continue;
	    }
	}
      else if (get_symbols_v2)
        get_symbols_v2 (info->handle, symtab->nsyms, syms);
      else
        get_symbols (info->handle, symtab->nsyms, syms);

      ++included_files;

      finish_conflict_resolution (symtab, &info->conflicts);
    }

  fprintf (f, "%d\n", included_files);

  for (i = 0; i < num_claimed_files; i++)
    {
      struct plugin_file_info *info = &claimed_files[i];
      struct plugin_symtab *symtab = &info->symtab;

      if (info->skip_file)
	continue;

      fprintf (f, "%s %d\n", info->name, symtab->nsyms + info->conflicts.nsyms);
      dump_symtab (f, symtab);
@@ -579,6 +603,7 @@ write_resolution (void)
	  free_symtab (&info->conflicts);
	}
    }

  fclose (f);
}

@@ -825,7 +850,8 @@ all_symbols_read_handler (void)
    {
      struct plugin_file_info *info = &claimed_files[i];

      *lto_arg_ptr++ = info->name;
      if (!info->skip_file)
	*lto_arg_ptr++ = info->name;
    }

  *lto_arg_ptr++ = NULL;
@@ -1402,6 +1428,9 @@ onload (struct ld_plugin_tv *tv)
	case LDPT_REGISTER_ALL_SYMBOLS_READ_HOOK:
	  register_all_symbols_read = p->tv_u.tv_register_all_symbols_read;
	  break;
	case LDPT_GET_SYMBOLS_V3:
	  get_symbols_v3 = p->tv_u.tv_get_symbols;
	  break;
	case LDPT_GET_SYMBOLS_V2:
	  get_symbols_v2 = p->tv_u.tv_get_symbols;
	  break;
