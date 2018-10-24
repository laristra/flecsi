
#include "one.hh"

// fun
int fun()
{
   // VisitVarDecl
   for (int i = 0;  i < 10;  ++i) {
      // VisitContinueStmt
      continue;
   }
   return 0;
}

// main
int main()
{
   // VisitTypeAliasDecl
   using Integer = int;

   // VisitVarDecl
   // VisitCallExpr
   int i = fun();
}
