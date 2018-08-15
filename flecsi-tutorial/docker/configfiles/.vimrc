"-------------------------------------------------------------------------------
" Vim configuration
"
" Useful commands:
"   :scriptnames - shows what scripts were executed in order
"-------------------------------------------------------------------------------

"-------------------------------------------------------------------------------
" General options.
"-------------------------------------------------------------------------------

set nocompatible
set history=100
set undolevels=1000
set showmatch
set showmode
set incsearch
set scrolloff=5
set ttyscroll=10
set ttyfast

"-------------------------------------------------------------------------------
" Enable syntax highlighting.
"-------------------------------------------------------------------------------

syntax on
set background=dark

"-------------------------------------------------------------------------------
" Turn on various filetype processing.
"-------------------------------------------------------------------------------

filetype on
filetype plugin on
filetype indent on

"-------------------------------------------------------------------------------
" C++ standard library headers.
"
" This list can be regenerated with:
"   $ for file in `find -maxdepth 1 -type f` ; do line=`echo $file |
"     sed 's,\.\/,,g'`; echo "  au! BufRead,BufNewFile" $line "set filetype=cpp"; done
"-------------------------------------------------------------------------------

augroup filetype
  au! BufRead,BufNewFile valarray set filetype=cpp
  au! BufRead,BufNewFile ctime set filetype=cpp
  au! BufRead,BufNewFile csetjmp set filetype=cpp
  au! BufRead,BufNewFile ctgmath set filetype=cpp
  au! BufRead,BufNewFile clocale set filetype=cpp
  au! BufRead,BufNewFile variant set filetype=cpp
  au! BufRead,BufNewFile cfenv set filetype=cpp
  au! BufRead,BufNewFile limits set filetype=cpp
  au! BufRead,BufNewFile ios set filetype=cpp
  au! BufRead,BufNewFile iterator set filetype=cpp
  au! BufRead,BufNewFile future set filetype=cpp
  au! BufRead,BufNewFile cstdalign set filetype=cpp
  au! BufRead,BufNewFile typeinfo set filetype=cpp
  au! BufRead,BufNewFile fenv.h set filetype=cpp
  au! BufRead,BufNewFile streambuf set filetype=cpp
  au! BufRead,BufNewFile regex set filetype=cpp
  au! BufRead,BufNewFile climits set filetype=cpp
  au! BufRead,BufNewFile ratio set filetype=cpp
  au! BufRead,BufNewFile istream set filetype=cpp
  au! BufRead,BufNewFile cstddef set filetype=cpp
  au! BufRead,BufNewFile fstream set filetype=cpp
  au! BufRead,BufNewFile ccomplex set filetype=cpp
  au! BufRead,BufNewFile new set filetype=cpp
  au! BufRead,BufNewFile condition_variable set filetype=cpp
  au! BufRead,BufNewFile tgmath.h set filetype=cpp
  au! BufRead,BufNewFile locale set filetype=cpp
  au! BufRead,BufNewFile bitset set filetype=cpp
  au! BufRead,BufNewFile tuple set filetype=cpp
  au! BufRead,BufNewFile cstdlib set filetype=cpp
  au! BufRead,BufNewFile deque set filetype=cpp
  au! BufRead,BufNewFile complex set filetype=cpp
  au! BufRead,BufNewFile string_view set filetype=cpp
  au! BufRead,BufNewFile forward_list set filetype=cpp
  au! BufRead,BufNewFile cctype set filetype=cpp
  au! BufRead,BufNewFile cassert set filetype=cpp
  au! BufRead,BufNewFile iomanip set filetype=cpp
  au! BufRead,BufNewFile stdlib.h set filetype=cpp
  au! BufRead,BufNewFile ostream set filetype=cpp
  au! BufRead,BufNewFile cstdio set filetype=cpp
  au! BufRead,BufNewFile type_traits set filetype=cpp
  au! BufRead,BufNewFile set set filetype=cpp
  au! BufRead,BufNewFile map set filetype=cpp
  au! BufRead,BufNewFile mutex set filetype=cpp
  au! BufRead,BufNewFile cmath set filetype=cpp
  au! BufRead,BufNewFile codecvt set filetype=cpp
  au! BufRead,BufNewFile cstdint set filetype=cpp
  au! BufRead,BufNewFile stdexcept set filetype=cpp
  au! BufRead,BufNewFile system_error set filetype=cpp
  au! BufRead,BufNewFile functional set filetype=cpp
  au! BufRead,BufNewFile csignal set filetype=cpp
  au! BufRead,BufNewFile cstdarg set filetype=cpp
  au! BufRead,BufNewFile sstream set filetype=cpp
  au! BufRead,BufNewFile cwctype set filetype=cpp
  au! BufRead,BufNewFile cxxabi.h set filetype=cpp
  au! BufRead,BufNewFile queue set filetype=cpp
  au! BufRead,BufNewFile cstring set filetype=cpp
  au! BufRead,BufNewFile random set filetype=cpp
  au! BufRead,BufNewFile cerrno set filetype=cpp
  au! BufRead,BufNewFile typeindex set filetype=cpp
  au! BufRead,BufNewFile any set filetype=cpp
  au! BufRead,BufNewFile array set filetype=cpp
  au! BufRead,BufNewFile utility set filetype=cpp
  au! BufRead,BufNewFile atomic set filetype=cpp
  au! BufRead,BufNewFile cwchar set filetype=cpp
  au! BufRead,BufNewFile memory set filetype=cpp
  au! BufRead,BufNewFile list set filetype=cpp
  au! BufRead,BufNewFile string set filetype=cpp
  au! BufRead,BufNewFile exception set filetype=cpp
  au! BufRead,BufNewFile cinttypes set filetype=cpp
  au! BufRead,BufNewFile complex.h set filetype=cpp
  au! BufRead,BufNewFile vector set filetype=cpp
  au! BufRead,BufNewFile scoped_allocator set filetype=cpp
  au! BufRead,BufNewFile numeric set filetype=cpp
  au! BufRead,BufNewFile unordered_map set filetype=cpp
  au! BufRead,BufNewFile cstdbool set filetype=cpp
  au! BufRead,BufNewFile math.h set filetype=cpp
  au! BufRead,BufNewFile iostream set filetype=cpp
  au! BufRead,BufNewFile shared_mutex set filetype=cpp
  au! BufRead,BufNewFile unordered_set set filetype=cpp
  au! BufRead,BufNewFile ciso646 set filetype=cpp
  au! BufRead,BufNewFile iosfwd set filetype=cpp
  au! BufRead,BufNewFile cfloat set filetype=cpp
  au! BufRead,BufNewFile initializer_list set filetype=cpp
  au! BufRead,BufNewFile optional set filetype=cpp
  au! BufRead,BufNewFile cuchar set filetype=cpp
  au! BufRead,BufNewFile thread set filetype=cpp
  au! BufRead,BufNewFile algorithm set filetype=cpp
  au! BufRead,BufNewFile stack set filetype=cpp
  au! BufRead,BufNewFile chrono set filetype=cpp
augroup END

"-------------------------------------------------------------------------------
" Control formatting of this file.
" vim: tabstop=2 shiftwidth=2 expandtab :
"-------------------------------------------------------------------------------
