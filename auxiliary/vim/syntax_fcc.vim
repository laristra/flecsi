" Vim syntax file
" Language: FleCSI C++
" Current Maintainer: Ben Bergen (https://github.com/ristra/flecsi)
" Last Change: 2018 Jan 23

" quit when a syntax file was already loaded
if exists("b:current_syntax")
  finish
endif

runtime! syntax/cpp.vim
unlet b:current_syntax

" FleCSI extensions
syn keyword flecsiRepeat forall reduceall scan

" Default highlighting
hi def link flecsiRepeat Repeat

let b:current_syntax = "fcc"
