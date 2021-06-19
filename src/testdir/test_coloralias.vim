" Tests for ":coloralias"

" source view_util.vim
" source screendump.vim
" source check.vim
" source script_util.vim

func Test_coloralias()
  " basic test if ":coloralias" doesn't crash
  coloralias rgb=#ffffff alias=white
  coloralias rgb=#ffeedd alias='a redish white'
  call assert_equal(0, len(v:errors))
endfunc

func Test_coloralias_invalid_syntax()
  call assert_fails("coloralias rgb=aabbcc", "Syntax error in coloralias")
  call assert_fails("coloralias alias='start of a col", "Broken quotes for alias=")
  call assert_fails("coloralias alias= rgb=", "Syntax error in coloralias")
endfunc

func Test_coloralias_required_args()
  call assert_fails('coloralias', 'Missing ')
  call assert_fails('coloralias alias=red', "Missing 'rgb='")
  call assert_fails('coloralias rgb=#ff0000', "Missing 'alias='")
endfunc

func Test_coloralias_highlight()
  coloralias rgb=#ffeedd alias='a redish white'
  highlight Normal guifg='a redish white'
  call assert_equal(0, len(v:errors))
endfunc

" vim: shiftwidth=2 sts=2 expandtab
