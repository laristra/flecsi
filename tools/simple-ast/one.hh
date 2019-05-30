
// VisitVarDecl (4)
int    i = 1;
int    j = 2;
float  f = 3.4;
double d = 5.6;

// foo, bar
#ifdef FOOBAR
   // VisitCXXRecordDecl
   struct foo
   {
   };

   // VisitCXXRecordDecl
   class bar
   {
   };
#endif
