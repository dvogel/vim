" Maintainer:  Drew Vogel <dvogel@sidejump.org>
" Last Change: 2021 Jul 25
"
" Replaced rgb.txt as the source of de facto standard color names. This is
" sourced each time the colorscheme command is run. It is also sourced each
" time the highlight command fails to recognize a gui color. You can override
" these colors by introducing a new colors/lists/default.vim file earlier in
" the runtimepath.

" Most users should use the `extend(v:colornames, { ... }, 'keep')` syntax to
" update v:colornames. This file uses the slower and more verbose syntax in
" order to retain vi compatibility.

if !has_key(v:colornames, 'snow')
    let v:colornames['snow'] = '#fffafa'
endif
if !has_key(v:colornames, 'ghost white')
    let v:colornames['ghost white'] = '#f8f8ff'
endif
if !has_key(v:colornames, 'ghostwhite')
    let v:colornames['ghostwhite'] = '#f8f8ff'
endif
if !has_key(v:colornames, 'white smoke')
    let v:colornames['white smoke'] = '#f5f5f5'
endif
if !has_key(v:colornames, 'whitesmoke')
    let v:colornames['whitesmoke'] = '#f5f5f5'
endif
if !has_key(v:colornames, 'gainsboro')
    let v:colornames['gainsboro'] = '#dcdcdc'
endif
if !has_key(v:colornames, 'floral white')
    let v:colornames['floral white'] = '#fffaf0'
endif
if !has_key(v:colornames, 'floralwhite')
    let v:colornames['floralwhite'] = '#fffaf0'
endif
if !has_key(v:colornames, 'old lace')
    let v:colornames['old lace'] = '#fdf5e6'
endif
if !has_key(v:colornames, 'oldlace')
    let v:colornames['oldlace'] = '#fdf5e6'
endif
if !has_key(v:colornames, 'linen')
    let v:colornames['linen'] = '#faf0e6'
endif
if !has_key(v:colornames, 'antique white')
    let v:colornames['antique white'] = '#faebd7'
endif
if !has_key(v:colornames, 'antiquewhite')
    let v:colornames['antiquewhite'] = '#faebd7'
endif
if !has_key(v:colornames, 'papaya whip')
    let v:colornames['papaya whip'] = '#ffefd5'
endif
if !has_key(v:colornames, 'papayawhip')
    let v:colornames['papayawhip'] = '#ffefd5'
endif
if !has_key(v:colornames, 'blanched almond')
    let v:colornames['blanched almond'] = '#ffebcd'
endif
if !has_key(v:colornames, 'blanchedalmond')
    let v:colornames['blanchedalmond'] = '#ffebcd'
endif
if !has_key(v:colornames, 'bisque')
    let v:colornames['bisque'] = '#ffe4c4'
endif
if !has_key(v:colornames, 'peach puff')
    let v:colornames['peach puff'] = '#ffdab9'
endif
if !has_key(v:colornames, 'peachpuff')
    let v:colornames['peachpuff'] = '#ffdab9'
endif
if !has_key(v:colornames, 'navajo white')
    let v:colornames['navajo white'] = '#ffdead'
endif
if !has_key(v:colornames, 'navajowhite')
    let v:colornames['navajowhite'] = '#ffdead'
endif
if !has_key(v:colornames, 'moccasin')
    let v:colornames['moccasin'] = '#ffe4b5'
endif
if !has_key(v:colornames, 'cornsilk')
    let v:colornames['cornsilk'] = '#fff8dc'
endif
if !has_key(v:colornames, 'ivory')
    let v:colornames['ivory'] = '#fffff0'
endif
if !has_key(v:colornames, 'lemon chiffon')
    let v:colornames['lemon chiffon'] = '#fffacd'
endif
if !has_key(v:colornames, 'lemonchiffon')
    let v:colornames['lemonchiffon'] = '#fffacd'
endif
if !has_key(v:colornames, 'seashell')
    let v:colornames['seashell'] = '#fff5ee'
endif
if !has_key(v:colornames, 'honeydew')
    let v:colornames['honeydew'] = '#f0fff0'
endif
if !has_key(v:colornames, 'mint cream')
    let v:colornames['mint cream'] = '#f5fffa'
endif
if !has_key(v:colornames, 'mintcream')
    let v:colornames['mintcream'] = '#f5fffa'
endif
if !has_key(v:colornames, 'azure')
    let v:colornames['azure'] = '#f0ffff'
endif
if !has_key(v:colornames, 'alice blue')
    let v:colornames['alice blue'] = '#f0f8ff'
endif
if !has_key(v:colornames, 'aliceblue')
    let v:colornames['aliceblue'] = '#f0f8ff'
endif
if !has_key(v:colornames, 'lavender')
    let v:colornames['lavender'] = '#e6e6fa'
endif
if !has_key(v:colornames, 'lavender blush')
    let v:colornames['lavender blush'] = '#fff0f5'
endif
if !has_key(v:colornames, 'lavenderblush')
    let v:colornames['lavenderblush'] = '#fff0f5'
endif
if !has_key(v:colornames, 'misty rose')
    let v:colornames['misty rose'] = '#ffe4e1'
endif
if !has_key(v:colornames, 'mistyrose')
    let v:colornames['mistyrose'] = '#ffe4e1'
endif
if !has_key(v:colornames, 'white')
    let v:colornames['white'] = '#ffffff'
endif
if !has_key(v:colornames, 'black')
    let v:colornames['black'] = '#000000'
endif
if !has_key(v:colornames, 'dark slate gray')
    let v:colornames['dark slate gray'] = '#2f4f4f'
endif
if !has_key(v:colornames, 'darkslategray')
    let v:colornames['darkslategray'] = '#2f4f4f'
endif
if !has_key(v:colornames, 'dark slate grey')
    let v:colornames['dark slate grey'] = '#2f4f4f'
endif
if !has_key(v:colornames, 'darkslategrey')
    let v:colornames['darkslategrey'] = '#2f4f4f'
endif
if !has_key(v:colornames, 'dim gray')
    let v:colornames['dim gray'] = '#696969'
endif
if !has_key(v:colornames, 'dimgray')
    let v:colornames['dimgray'] = '#696969'
endif
if !has_key(v:colornames, 'dim grey')
    let v:colornames['dim grey'] = '#696969'
endif
if !has_key(v:colornames, 'dimgrey')
    let v:colornames['dimgrey'] = '#696969'
endif
if !has_key(v:colornames, 'slate gray')
    let v:colornames['slate gray'] = '#708090'
endif
if !has_key(v:colornames, 'slategray')
    let v:colornames['slategray'] = '#708090'
endif
if !has_key(v:colornames, 'slate grey')
    let v:colornames['slate grey'] = '#708090'
endif
if !has_key(v:colornames, 'slategrey')
    let v:colornames['slategrey'] = '#708090'
endif
if !has_key(v:colornames, 'light slate gray')
    let v:colornames['light slate gray'] = '#778899'
endif
if !has_key(v:colornames, 'lightslategray')
    let v:colornames['lightslategray'] = '#778899'
endif
if !has_key(v:colornames, 'light slate grey')
    let v:colornames['light slate grey'] = '#778899'
endif
if !has_key(v:colornames, 'lightslategrey')
    let v:colornames['lightslategrey'] = '#778899'
endif
if !has_key(v:colornames, 'gray')
    let v:colornames['gray'] = '#bebebe'
endif
if !has_key(v:colornames, 'grey')
    let v:colornames['grey'] = '#bebebe'
endif
if !has_key(v:colornames, 'x11 gray')
    let v:colornames['x11 gray'] = '#bebebe'
endif
if !has_key(v:colornames, 'x11gray')
    let v:colornames['x11gray'] = '#bebebe'
endif
if !has_key(v:colornames, 'x11 grey')
    let v:colornames['x11 grey'] = '#bebebe'
endif
if !has_key(v:colornames, 'x11grey')
    let v:colornames['x11grey'] = '#bebebe'
endif
if !has_key(v:colornames, 'web gray')
    let v:colornames['web gray'] = '#808080'
endif
if !has_key(v:colornames, 'webgray')
    let v:colornames['webgray'] = '#808080'
endif
if !has_key(v:colornames, 'web grey')
    let v:colornames['web grey'] = '#808080'
endif
if !has_key(v:colornames, 'webgrey')
    let v:colornames['webgrey'] = '#808080'
endif
if !has_key(v:colornames, 'light grey')
    let v:colornames['light grey'] = '#d3d3d3'
endif
if !has_key(v:colornames, 'lightgrey')
    let v:colornames['lightgrey'] = '#d3d3d3'
endif
if !has_key(v:colornames, 'light gray')
    let v:colornames['light gray'] = '#d3d3d3'
endif
if !has_key(v:colornames, 'lightgray')
    let v:colornames['lightgray'] = '#d3d3d3'
endif
if !has_key(v:colornames, 'midnight blue')
    let v:colornames['midnight blue'] = '#191970'
endif
if !has_key(v:colornames, 'midnightblue')
    let v:colornames['midnightblue'] = '#191970'
endif
if !has_key(v:colornames, 'navy')
    let v:colornames['navy'] = '#000080'
endif
if !has_key(v:colornames, 'navy blue')
    let v:colornames['navy blue'] = '#000080'
endif
if !has_key(v:colornames, 'navyblue')
    let v:colornames['navyblue'] = '#000080'
endif
if !has_key(v:colornames, 'cornflower blue')
    let v:colornames['cornflower blue'] = '#6495ed'
endif
if !has_key(v:colornames, 'cornflowerblue')
    let v:colornames['cornflowerblue'] = '#6495ed'
endif
if !has_key(v:colornames, 'dark slate blue')
    let v:colornames['dark slate blue'] = '#483d8b'
endif
if !has_key(v:colornames, 'darkslateblue')
    let v:colornames['darkslateblue'] = '#483d8b'
endif
if !has_key(v:colornames, 'slate blue')
    let v:colornames['slate blue'] = '#6a5acd'
endif
if !has_key(v:colornames, 'slateblue')
    let v:colornames['slateblue'] = '#6a5acd'
endif
if !has_key(v:colornames, 'medium slate blue')
    let v:colornames['medium slate blue'] = '#7b68ee'
endif
if !has_key(v:colornames, 'mediumslateblue')
    let v:colornames['mediumslateblue'] = '#7b68ee'
endif
if !has_key(v:colornames, 'light slate blue')
    let v:colornames['light slate blue'] = '#8470ff'
endif
if !has_key(v:colornames, 'lightslateblue')
    let v:colornames['lightslateblue'] = '#8470ff'
endif
if !has_key(v:colornames, 'medium blue')
    let v:colornames['medium blue'] = '#0000cd'
endif
if !has_key(v:colornames, 'mediumblue')
    let v:colornames['mediumblue'] = '#0000cd'
endif
if !has_key(v:colornames, 'royal blue')
    let v:colornames['royal blue'] = '#4169e1'
endif
if !has_key(v:colornames, 'royalblue')
    let v:colornames['royalblue'] = '#4169e1'
endif
if !has_key(v:colornames, 'blue')
    let v:colornames['blue'] = '#0000ff'
endif
if !has_key(v:colornames, 'dodger blue')
    let v:colornames['dodger blue'] = '#1e90ff'
endif
if !has_key(v:colornames, 'dodgerblue')
    let v:colornames['dodgerblue'] = '#1e90ff'
endif
if !has_key(v:colornames, 'deep sky blue')
    let v:colornames['deep sky blue'] = '#00bfff'
endif
if !has_key(v:colornames, 'deepskyblue')
    let v:colornames['deepskyblue'] = '#00bfff'
endif
if !has_key(v:colornames, 'sky blue')
    let v:colornames['sky blue'] = '#87ceeb'
endif
if !has_key(v:colornames, 'skyblue')
    let v:colornames['skyblue'] = '#87ceeb'
endif
if !has_key(v:colornames, 'light sky blue')
    let v:colornames['light sky blue'] = '#87cefa'
endif
if !has_key(v:colornames, 'lightskyblue')
    let v:colornames['lightskyblue'] = '#87cefa'
endif
if !has_key(v:colornames, 'steel blue')
    let v:colornames['steel blue'] = '#4682b4'
endif
if !has_key(v:colornames, 'steelblue')
    let v:colornames['steelblue'] = '#4682b4'
endif
if !has_key(v:colornames, 'light steel blue')
    let v:colornames['light steel blue'] = '#b0c4de'
endif
if !has_key(v:colornames, 'lightsteelblue')
    let v:colornames['lightsteelblue'] = '#b0c4de'
endif
if !has_key(v:colornames, 'light blue')
    let v:colornames['light blue'] = '#add8e6'
endif
if !has_key(v:colornames, 'lightblue')
    let v:colornames['lightblue'] = '#add8e6'
endif
if !has_key(v:colornames, 'powder blue')
    let v:colornames['powder blue'] = '#b0e0e6'
endif
if !has_key(v:colornames, 'powderblue')
    let v:colornames['powderblue'] = '#b0e0e6'
endif
if !has_key(v:colornames, 'pale turquoise')
    let v:colornames['pale turquoise'] = '#afeeee'
endif
if !has_key(v:colornames, 'paleturquoise')
    let v:colornames['paleturquoise'] = '#afeeee'
endif
if !has_key(v:colornames, 'dark turquoise')
    let v:colornames['dark turquoise'] = '#00ced1'
endif
if !has_key(v:colornames, 'darkturquoise')
    let v:colornames['darkturquoise'] = '#00ced1'
endif
if !has_key(v:colornames, 'medium turquoise')
    let v:colornames['medium turquoise'] = '#48d1cc'
endif
if !has_key(v:colornames, 'mediumturquoise')
    let v:colornames['mediumturquoise'] = '#48d1cc'
endif
if !has_key(v:colornames, 'turquoise')
    let v:colornames['turquoise'] = '#40e0d0'
endif
if !has_key(v:colornames, 'cyan')
    let v:colornames['cyan'] = '#00ffff'
endif
if !has_key(v:colornames, 'aqua')
    let v:colornames['aqua'] = '#00ffff'
endif
if !has_key(v:colornames, 'light cyan')
    let v:colornames['light cyan'] = '#e0ffff'
endif
if !has_key(v:colornames, 'lightcyan')
    let v:colornames['lightcyan'] = '#e0ffff'
endif
if !has_key(v:colornames, 'cadet blue')
    let v:colornames['cadet blue'] = '#5f9ea0'
endif
if !has_key(v:colornames, 'cadetblue')
    let v:colornames['cadetblue'] = '#5f9ea0'
endif
if !has_key(v:colornames, 'medium aquamarine')
    let v:colornames['medium aquamarine'] = '#66cdaa'
endif
if !has_key(v:colornames, 'mediumaquamarine')
    let v:colornames['mediumaquamarine'] = '#66cdaa'
endif
if !has_key(v:colornames, 'aquamarine')
    let v:colornames['aquamarine'] = '#7fffd4'
endif
if !has_key(v:colornames, 'dark green')
    let v:colornames['dark green'] = '#006400'
endif
if !has_key(v:colornames, 'darkgreen')
    let v:colornames['darkgreen'] = '#006400'
endif
if !has_key(v:colornames, 'dark olive green')
    let v:colornames['dark olive green'] = '#556b2f'
endif
if !has_key(v:colornames, 'darkolivegreen')
    let v:colornames['darkolivegreen'] = '#556b2f'
endif
if !has_key(v:colornames, 'dark sea green')
    let v:colornames['dark sea green'] = '#8fbc8f'
endif
if !has_key(v:colornames, 'darkseagreen')
    let v:colornames['darkseagreen'] = '#8fbc8f'
endif
if !has_key(v:colornames, 'sea green')
    let v:colornames['sea green'] = '#2e8b57'
endif
if !has_key(v:colornames, 'seagreen')
    let v:colornames['seagreen'] = '#2e8b57'
endif
if !has_key(v:colornames, 'medium sea green')
    let v:colornames['medium sea green'] = '#3cb371'
endif
if !has_key(v:colornames, 'mediumseagreen')
    let v:colornames['mediumseagreen'] = '#3cb371'
endif
if !has_key(v:colornames, 'light sea green')
    let v:colornames['light sea green'] = '#20b2aa'
endif
if !has_key(v:colornames, 'lightseagreen')
    let v:colornames['lightseagreen'] = '#20b2aa'
endif
if !has_key(v:colornames, 'pale green')
    let v:colornames['pale green'] = '#98fb98'
endif
if !has_key(v:colornames, 'palegreen')
    let v:colornames['palegreen'] = '#98fb98'
endif
if !has_key(v:colornames, 'spring green')
    let v:colornames['spring green'] = '#00ff7f'
endif
if !has_key(v:colornames, 'springgreen')
    let v:colornames['springgreen'] = '#00ff7f'
endif
if !has_key(v:colornames, 'lawn green')
    let v:colornames['lawn green'] = '#7cfc00'
endif
if !has_key(v:colornames, 'lawngreen')
    let v:colornames['lawngreen'] = '#7cfc00'
endif
if !has_key(v:colornames, 'green')
    let v:colornames['green'] = '#00ff00'
endif
if !has_key(v:colornames, 'lime')
    let v:colornames['lime'] = '#00ff00'
endif
if !has_key(v:colornames, 'x11 green')
    let v:colornames['x11 green'] = '#00ff00'
endif
if !has_key(v:colornames, 'x11green')
    let v:colornames['x11green'] = '#00ff00'
endif
if !has_key(v:colornames, 'web green')
    let v:colornames['web green'] = '#008000'
endif
if !has_key(v:colornames, 'webgreen')
    let v:colornames['webgreen'] = '#008000'
endif
if !has_key(v:colornames, 'chartreuse')
    let v:colornames['chartreuse'] = '#7fff00'
endif
if !has_key(v:colornames, 'medium spring green')
    let v:colornames['medium spring green'] = '#00fa9a'
endif
if !has_key(v:colornames, 'mediumspringgreen')
    let v:colornames['mediumspringgreen'] = '#00fa9a'
endif
if !has_key(v:colornames, 'green yellow')
    let v:colornames['green yellow'] = '#adff2f'
endif
if !has_key(v:colornames, 'greenyellow')
    let v:colornames['greenyellow'] = '#adff2f'
endif
if !has_key(v:colornames, 'lime green')
    let v:colornames['lime green'] = '#32cd32'
endif
if !has_key(v:colornames, 'limegreen')
    let v:colornames['limegreen'] = '#32cd32'
endif
if !has_key(v:colornames, 'yellow green')
    let v:colornames['yellow green'] = '#9acd32'
endif
if !has_key(v:colornames, 'yellowgreen')
    let v:colornames['yellowgreen'] = '#9acd32'
endif
if !has_key(v:colornames, 'forest green')
    let v:colornames['forest green'] = '#228b22'
endif
if !has_key(v:colornames, 'forestgreen')
    let v:colornames['forestgreen'] = '#228b22'
endif
if !has_key(v:colornames, 'olive drab')
    let v:colornames['olive drab'] = '#6b8e23'
endif
if !has_key(v:colornames, 'olivedrab')
    let v:colornames['olivedrab'] = '#6b8e23'
endif
if !has_key(v:colornames, 'dark khaki')
    let v:colornames['dark khaki'] = '#bdb76b'
endif
if !has_key(v:colornames, 'darkkhaki')
    let v:colornames['darkkhaki'] = '#bdb76b'
endif
if !has_key(v:colornames, 'khaki')
    let v:colornames['khaki'] = '#f0e68c'
endif
if !has_key(v:colornames, 'pale goldenrod')
    let v:colornames['pale goldenrod'] = '#eee8aa'
endif
if !has_key(v:colornames, 'palegoldenrod')
    let v:colornames['palegoldenrod'] = '#eee8aa'
endif
if !has_key(v:colornames, 'light goldenrod yellow')
    let v:colornames['light goldenrod yellow'] = '#fafad2'
endif
if !has_key(v:colornames, 'lightgoldenrodyellow')
    let v:colornames['lightgoldenrodyellow'] = '#fafad2'
endif
if !has_key(v:colornames, 'light yellow')
    let v:colornames['light yellow'] = '#ffffe0'
endif
if !has_key(v:colornames, 'lightyellow')
    let v:colornames['lightyellow'] = '#ffffe0'
endif
if !has_key(v:colornames, 'yellow')
    let v:colornames['yellow'] = '#ffff00'
endif
if !has_key(v:colornames, 'gold')
    let v:colornames['gold'] = '#ffd700'
endif
if !has_key(v:colornames, 'light goldenrod')
    let v:colornames['light goldenrod'] = '#eedd82'
endif
if !has_key(v:colornames, 'lightgoldenrod')
    let v:colornames['lightgoldenrod'] = '#eedd82'
endif
if !has_key(v:colornames, 'goldenrod')
    let v:colornames['goldenrod'] = '#daa520'
endif
if !has_key(v:colornames, 'dark goldenrod')
    let v:colornames['dark goldenrod'] = '#b8860b'
endif
if !has_key(v:colornames, 'darkgoldenrod')
    let v:colornames['darkgoldenrod'] = '#b8860b'
endif
if !has_key(v:colornames, 'rosy brown')
    let v:colornames['rosy brown'] = '#bc8f8f'
endif
if !has_key(v:colornames, 'rosybrown')
    let v:colornames['rosybrown'] = '#bc8f8f'
endif
if !has_key(v:colornames, 'indian red')
    let v:colornames['indian red'] = '#cd5c5c'
endif
if !has_key(v:colornames, 'indianred')
    let v:colornames['indianred'] = '#cd5c5c'
endif
if !has_key(v:colornames, 'saddle brown')
    let v:colornames['saddle brown'] = '#8b4513'
endif
if !has_key(v:colornames, 'saddlebrown')
    let v:colornames['saddlebrown'] = '#8b4513'
endif
if !has_key(v:colornames, 'sienna')
    let v:colornames['sienna'] = '#a0522d'
endif
if !has_key(v:colornames, 'peru')
    let v:colornames['peru'] = '#cd853f'
endif
if !has_key(v:colornames, 'burlywood')
    let v:colornames['burlywood'] = '#deb887'
endif
if !has_key(v:colornames, 'beige')
    let v:colornames['beige'] = '#f5f5dc'
endif
if !has_key(v:colornames, 'wheat')
    let v:colornames['wheat'] = '#f5deb3'
endif
if !has_key(v:colornames, 'sandy brown')
    let v:colornames['sandy brown'] = '#f4a460'
endif
if !has_key(v:colornames, 'sandybrown')
    let v:colornames['sandybrown'] = '#f4a460'
endif
if !has_key(v:colornames, 'tan')
    let v:colornames['tan'] = '#d2b48c'
endif
if !has_key(v:colornames, 'chocolate')
    let v:colornames['chocolate'] = '#d2691e'
endif
if !has_key(v:colornames, 'firebrick')
    let v:colornames['firebrick'] = '#b22222'
endif
if !has_key(v:colornames, 'brown')
    let v:colornames['brown'] = '#a52a2a'
endif
if !has_key(v:colornames, 'dark salmon')
    let v:colornames['dark salmon'] = '#e9967a'
endif
if !has_key(v:colornames, 'darksalmon')
    let v:colornames['darksalmon'] = '#e9967a'
endif
if !has_key(v:colornames, 'salmon')
    let v:colornames['salmon'] = '#fa8072'
endif
if !has_key(v:colornames, 'light salmon')
    let v:colornames['light salmon'] = '#ffa07a'
endif
if !has_key(v:colornames, 'lightsalmon')
    let v:colornames['lightsalmon'] = '#ffa07a'
endif
if !has_key(v:colornames, 'orange')
    let v:colornames['orange'] = '#ffa500'
endif
if !has_key(v:colornames, 'dark orange')
    let v:colornames['dark orange'] = '#ff8c00'
endif
if !has_key(v:colornames, 'darkorange')
    let v:colornames['darkorange'] = '#ff8c00'
endif
if !has_key(v:colornames, 'coral')
    let v:colornames['coral'] = '#ff7f50'
endif
if !has_key(v:colornames, 'light coral')
    let v:colornames['light coral'] = '#f08080'
endif
if !has_key(v:colornames, 'lightcoral')
    let v:colornames['lightcoral'] = '#f08080'
endif
if !has_key(v:colornames, 'tomato')
    let v:colornames['tomato'] = '#ff6347'
endif
if !has_key(v:colornames, 'orange red')
    let v:colornames['orange red'] = '#ff4500'
endif
if !has_key(v:colornames, 'orangered')
    let v:colornames['orangered'] = '#ff4500'
endif
if !has_key(v:colornames, 'red')
    let v:colornames['red'] = '#ff0000'
endif
if !has_key(v:colornames, 'hot pink')
    let v:colornames['hot pink'] = '#ff69b4'
endif
if !has_key(v:colornames, 'hotpink')
    let v:colornames['hotpink'] = '#ff69b4'
endif
if !has_key(v:colornames, 'deep pink')
    let v:colornames['deep pink'] = '#ff1493'
endif
if !has_key(v:colornames, 'deeppink')
    let v:colornames['deeppink'] = '#ff1493'
endif
if !has_key(v:colornames, 'pink')
    let v:colornames['pink'] = '#ffc0cb'
endif
if !has_key(v:colornames, 'light pink')
    let v:colornames['light pink'] = '#ffb6c1'
endif
if !has_key(v:colornames, 'lightpink')
    let v:colornames['lightpink'] = '#ffb6c1'
endif
if !has_key(v:colornames, 'pale violet red')
    let v:colornames['pale violet red'] = '#db7093'
endif
if !has_key(v:colornames, 'palevioletred')
    let v:colornames['palevioletred'] = '#db7093'
endif
if !has_key(v:colornames, 'maroon')
    let v:colornames['maroon'] = '#b03060'
endif
if !has_key(v:colornames, 'x11 maroon')
    let v:colornames['x11 maroon'] = '#b03060'
endif
if !has_key(v:colornames, 'x11maroon')
    let v:colornames['x11maroon'] = '#b03060'
endif
if !has_key(v:colornames, 'web maroon')
    let v:colornames['web maroon'] = '#800000'
endif
if !has_key(v:colornames, 'webmaroon')
    let v:colornames['webmaroon'] = '#800000'
endif
if !has_key(v:colornames, 'medium violet red')
    let v:colornames['medium violet red'] = '#c71585'
endif
if !has_key(v:colornames, 'mediumvioletred')
    let v:colornames['mediumvioletred'] = '#c71585'
endif
if !has_key(v:colornames, 'violet red')
    let v:colornames['violet red'] = '#d02090'
endif
if !has_key(v:colornames, 'violetred')
    let v:colornames['violetred'] = '#d02090'
endif
if !has_key(v:colornames, 'magenta')
    let v:colornames['magenta'] = '#ff00ff'
endif
if !has_key(v:colornames, 'fuchsia')
    let v:colornames['fuchsia'] = '#ff00ff'
endif
if !has_key(v:colornames, 'violet')
    let v:colornames['violet'] = '#ee82ee'
endif
if !has_key(v:colornames, 'plum')
    let v:colornames['plum'] = '#dda0dd'
endif
if !has_key(v:colornames, 'orchid')
    let v:colornames['orchid'] = '#da70d6'
endif
if !has_key(v:colornames, 'medium orchid')
    let v:colornames['medium orchid'] = '#ba55d3'
endif
if !has_key(v:colornames, 'mediumorchid')
    let v:colornames['mediumorchid'] = '#ba55d3'
endif
if !has_key(v:colornames, 'dark orchid')
    let v:colornames['dark orchid'] = '#9932cc'
endif
if !has_key(v:colornames, 'darkorchid')
    let v:colornames['darkorchid'] = '#9932cc'
endif
if !has_key(v:colornames, 'dark violet')
    let v:colornames['dark violet'] = '#9400d3'
endif
if !has_key(v:colornames, 'darkviolet')
    let v:colornames['darkviolet'] = '#9400d3'
endif
if !has_key(v:colornames, 'blue violet')
    let v:colornames['blue violet'] = '#8a2be2'
endif
if !has_key(v:colornames, 'blueviolet')
    let v:colornames['blueviolet'] = '#8a2be2'
endif
if !has_key(v:colornames, 'purple')
    let v:colornames['purple'] = '#a020f0'
endif
if !has_key(v:colornames, 'x11 purple')
    let v:colornames['x11 purple'] = '#a020f0'
endif
if !has_key(v:colornames, 'x11purple')
    let v:colornames['x11purple'] = '#a020f0'
endif
if !has_key(v:colornames, 'web purple')
    let v:colornames['web purple'] = '#800080'
endif
if !has_key(v:colornames, 'webpurple')
    let v:colornames['webpurple'] = '#800080'
endif
if !has_key(v:colornames, 'medium purple')
    let v:colornames['medium purple'] = '#9370db'
endif
if !has_key(v:colornames, 'mediumpurple')
    let v:colornames['mediumpurple'] = '#9370db'
endif
if !has_key(v:colornames, 'thistle')
    let v:colornames['thistle'] = '#d8bfd8'
endif
if !has_key(v:colornames, 'snow1')
    let v:colornames['snow1'] = '#fffafa'
endif
if !has_key(v:colornames, 'snow2')
    let v:colornames['snow2'] = '#eee9e9'
endif
if !has_key(v:colornames, 'snow3')
    let v:colornames['snow3'] = '#cdc9c9'
endif
if !has_key(v:colornames, 'snow4')
    let v:colornames['snow4'] = '#8b8989'
endif
if !has_key(v:colornames, 'seashell1')
    let v:colornames['seashell1'] = '#fff5ee'
endif
if !has_key(v:colornames, 'seashell2')
    let v:colornames['seashell2'] = '#eee5de'
endif
if !has_key(v:colornames, 'seashell3')
    let v:colornames['seashell3'] = '#cdc5bf'
endif
if !has_key(v:colornames, 'seashell4')
    let v:colornames['seashell4'] = '#8b8682'
endif
if !has_key(v:colornames, 'antiquewhite1')
    let v:colornames['antiquewhite1'] = '#ffefdb'
endif
if !has_key(v:colornames, 'antiquewhite2')
    let v:colornames['antiquewhite2'] = '#eedfcc'
endif
if !has_key(v:colornames, 'antiquewhite3')
    let v:colornames['antiquewhite3'] = '#cdc0b0'
endif
if !has_key(v:colornames, 'antiquewhite4')
    let v:colornames['antiquewhite4'] = '#8b8378'
endif
if !has_key(v:colornames, 'bisque1')
    let v:colornames['bisque1'] = '#ffe4c4'
endif
if !has_key(v:colornames, 'bisque2')
    let v:colornames['bisque2'] = '#eed5b7'
endif
if !has_key(v:colornames, 'bisque3')
    let v:colornames['bisque3'] = '#cdb79e'
endif
if !has_key(v:colornames, 'bisque4')
    let v:colornames['bisque4'] = '#8b7d6b'
endif
if !has_key(v:colornames, 'peachpuff1')
    let v:colornames['peachpuff1'] = '#ffdab9'
endif
if !has_key(v:colornames, 'peachpuff2')
    let v:colornames['peachpuff2'] = '#eecbad'
endif
if !has_key(v:colornames, 'peachpuff3')
    let v:colornames['peachpuff3'] = '#cdaf95'
endif
if !has_key(v:colornames, 'peachpuff4')
    let v:colornames['peachpuff4'] = '#8b7765'
endif
if !has_key(v:colornames, 'navajowhite1')
    let v:colornames['navajowhite1'] = '#ffdead'
endif
if !has_key(v:colornames, 'navajowhite2')
    let v:colornames['navajowhite2'] = '#eecfa1'
endif
if !has_key(v:colornames, 'navajowhite3')
    let v:colornames['navajowhite3'] = '#cdb38b'
endif
if !has_key(v:colornames, 'navajowhite4')
    let v:colornames['navajowhite4'] = '#8b795e'
endif
if !has_key(v:colornames, 'lemonchiffon1')
    let v:colornames['lemonchiffon1'] = '#fffacd'
endif
if !has_key(v:colornames, 'lemonchiffon2')
    let v:colornames['lemonchiffon2'] = '#eee9bf'
endif
if !has_key(v:colornames, 'lemonchiffon3')
    let v:colornames['lemonchiffon3'] = '#cdc9a5'
endif
if !has_key(v:colornames, 'lemonchiffon4')
    let v:colornames['lemonchiffon4'] = '#8b8970'
endif
if !has_key(v:colornames, 'cornsilk1')
    let v:colornames['cornsilk1'] = '#fff8dc'
endif
if !has_key(v:colornames, 'cornsilk2')
    let v:colornames['cornsilk2'] = '#eee8cd'
endif
if !has_key(v:colornames, 'cornsilk3')
    let v:colornames['cornsilk3'] = '#cdc8b1'
endif
if !has_key(v:colornames, 'cornsilk4')
    let v:colornames['cornsilk4'] = '#8b8878'
endif
if !has_key(v:colornames, 'ivory1')
    let v:colornames['ivory1'] = '#fffff0'
endif
if !has_key(v:colornames, 'ivory2')
    let v:colornames['ivory2'] = '#eeeee0'
endif
if !has_key(v:colornames, 'ivory3')
    let v:colornames['ivory3'] = '#cdcdc1'
endif
if !has_key(v:colornames, 'ivory4')
    let v:colornames['ivory4'] = '#8b8b83'
endif
if !has_key(v:colornames, 'honeydew1')
    let v:colornames['honeydew1'] = '#f0fff0'
endif
if !has_key(v:colornames, 'honeydew2')
    let v:colornames['honeydew2'] = '#e0eee0'
endif
if !has_key(v:colornames, 'honeydew3')
    let v:colornames['honeydew3'] = '#c1cdc1'
endif
if !has_key(v:colornames, 'honeydew4')
    let v:colornames['honeydew4'] = '#838b83'
endif
if !has_key(v:colornames, 'lavenderblush1')
    let v:colornames['lavenderblush1'] = '#fff0f5'
endif
if !has_key(v:colornames, 'lavenderblush2')
    let v:colornames['lavenderblush2'] = '#eee0e5'
endif
if !has_key(v:colornames, 'lavenderblush3')
    let v:colornames['lavenderblush3'] = '#cdc1c5'
endif
if !has_key(v:colornames, 'lavenderblush4')
    let v:colornames['lavenderblush4'] = '#8b8386'
endif
if !has_key(v:colornames, 'mistyrose1')
    let v:colornames['mistyrose1'] = '#ffe4e1'
endif
if !has_key(v:colornames, 'mistyrose2')
    let v:colornames['mistyrose2'] = '#eed5d2'
endif
if !has_key(v:colornames, 'mistyrose3')
    let v:colornames['mistyrose3'] = '#cdb7b5'
endif
if !has_key(v:colornames, 'mistyrose4')
    let v:colornames['mistyrose4'] = '#8b7d7b'
endif
if !has_key(v:colornames, 'azure1')
    let v:colornames['azure1'] = '#f0ffff'
endif
if !has_key(v:colornames, 'azure2')
    let v:colornames['azure2'] = '#e0eeee'
endif
if !has_key(v:colornames, 'azure3')
    let v:colornames['azure3'] = '#c1cdcd'
endif
if !has_key(v:colornames, 'azure4')
    let v:colornames['azure4'] = '#838b8b'
endif
if !has_key(v:colornames, 'slateblue1')
    let v:colornames['slateblue1'] = '#836fff'
endif
if !has_key(v:colornames, 'slateblue2')
    let v:colornames['slateblue2'] = '#7a67ee'
endif
if !has_key(v:colornames, 'slateblue3')
    let v:colornames['slateblue3'] = '#6959cd'
endif
if !has_key(v:colornames, 'slateblue4')
    let v:colornames['slateblue4'] = '#473c8b'
endif
if !has_key(v:colornames, 'royalblue1')
    let v:colornames['royalblue1'] = '#4876ff'
endif
if !has_key(v:colornames, 'royalblue2')
    let v:colornames['royalblue2'] = '#436eee'
endif
if !has_key(v:colornames, 'royalblue3')
    let v:colornames['royalblue3'] = '#3a5fcd'
endif
if !has_key(v:colornames, 'royalblue4')
    let v:colornames['royalblue4'] = '#27408b'
endif
if !has_key(v:colornames, 'blue1')
    let v:colornames['blue1'] = '#0000ff'
endif
if !has_key(v:colornames, 'blue2')
    let v:colornames['blue2'] = '#0000ee'
endif
if !has_key(v:colornames, 'blue3')
    let v:colornames['blue3'] = '#0000cd'
endif
if !has_key(v:colornames, 'blue4')
    let v:colornames['blue4'] = '#00008b'
endif
if !has_key(v:colornames, 'dodgerblue1')
    let v:colornames['dodgerblue1'] = '#1e90ff'
endif
if !has_key(v:colornames, 'dodgerblue2')
    let v:colornames['dodgerblue2'] = '#1c86ee'
endif
if !has_key(v:colornames, 'dodgerblue3')
    let v:colornames['dodgerblue3'] = '#1874cd'
endif
if !has_key(v:colornames, 'dodgerblue4')
    let v:colornames['dodgerblue4'] = '#104e8b'
endif
if !has_key(v:colornames, 'steelblue1')
    let v:colornames['steelblue1'] = '#63b8ff'
endif
if !has_key(v:colornames, 'steelblue2')
    let v:colornames['steelblue2'] = '#5cacee'
endif
if !has_key(v:colornames, 'steelblue3')
    let v:colornames['steelblue3'] = '#4f94cd'
endif
if !has_key(v:colornames, 'steelblue4')
    let v:colornames['steelblue4'] = '#36648b'
endif
if !has_key(v:colornames, 'deepskyblue1')
    let v:colornames['deepskyblue1'] = '#00bfff'
endif
if !has_key(v:colornames, 'deepskyblue2')
    let v:colornames['deepskyblue2'] = '#00b2ee'
endif
if !has_key(v:colornames, 'deepskyblue3')
    let v:colornames['deepskyblue3'] = '#009acd'
endif
if !has_key(v:colornames, 'deepskyblue4')
    let v:colornames['deepskyblue4'] = '#00688b'
endif
if !has_key(v:colornames, 'skyblue1')
    let v:colornames['skyblue1'] = '#87ceff'
endif
if !has_key(v:colornames, 'skyblue2')
    let v:colornames['skyblue2'] = '#7ec0ee'
endif
if !has_key(v:colornames, 'skyblue3')
    let v:colornames['skyblue3'] = '#6ca6cd'
endif
if !has_key(v:colornames, 'skyblue4')
    let v:colornames['skyblue4'] = '#4a708b'
endif
if !has_key(v:colornames, 'lightskyblue1')
    let v:colornames['lightskyblue1'] = '#b0e2ff'
endif
if !has_key(v:colornames, 'lightskyblue2')
    let v:colornames['lightskyblue2'] = '#a4d3ee'
endif
if !has_key(v:colornames, 'lightskyblue3')
    let v:colornames['lightskyblue3'] = '#8db6cd'
endif
if !has_key(v:colornames, 'lightskyblue4')
    let v:colornames['lightskyblue4'] = '#607b8b'
endif
if !has_key(v:colornames, 'slategray1')
    let v:colornames['slategray1'] = '#c6e2ff'
endif
if !has_key(v:colornames, 'slategray2')
    let v:colornames['slategray2'] = '#b9d3ee'
endif
if !has_key(v:colornames, 'slategray3')
    let v:colornames['slategray3'] = '#9fb6cd'
endif
if !has_key(v:colornames, 'slategray4')
    let v:colornames['slategray4'] = '#6c7b8b'
endif
if !has_key(v:colornames, 'lightsteelblue1')
    let v:colornames['lightsteelblue1'] = '#cae1ff'
endif
if !has_key(v:colornames, 'lightsteelblue2')
    let v:colornames['lightsteelblue2'] = '#bcd2ee'
endif
if !has_key(v:colornames, 'lightsteelblue3')
    let v:colornames['lightsteelblue3'] = '#a2b5cd'
endif
if !has_key(v:colornames, 'lightsteelblue4')
    let v:colornames['lightsteelblue4'] = '#6e7b8b'
endif
if !has_key(v:colornames, 'lightblue1')
    let v:colornames['lightblue1'] = '#bfefff'
endif
if !has_key(v:colornames, 'lightblue2')
    let v:colornames['lightblue2'] = '#b2dfee'
endif
if !has_key(v:colornames, 'lightblue3')
    let v:colornames['lightblue3'] = '#9ac0cd'
endif
if !has_key(v:colornames, 'lightblue4')
    let v:colornames['lightblue4'] = '#68838b'
endif
if !has_key(v:colornames, 'lightcyan1')
    let v:colornames['lightcyan1'] = '#e0ffff'
endif
if !has_key(v:colornames, 'lightcyan2')
    let v:colornames['lightcyan2'] = '#d1eeee'
endif
if !has_key(v:colornames, 'lightcyan3')
    let v:colornames['lightcyan3'] = '#b4cdcd'
endif
if !has_key(v:colornames, 'lightcyan4')
    let v:colornames['lightcyan4'] = '#7a8b8b'
endif
if !has_key(v:colornames, 'paleturquoise1')
    let v:colornames['paleturquoise1'] = '#bbffff'
endif
if !has_key(v:colornames, 'paleturquoise2')
    let v:colornames['paleturquoise2'] = '#aeeeee'
endif
if !has_key(v:colornames, 'paleturquoise3')
    let v:colornames['paleturquoise3'] = '#96cdcd'
endif
if !has_key(v:colornames, 'paleturquoise4')
    let v:colornames['paleturquoise4'] = '#668b8b'
endif
if !has_key(v:colornames, 'cadetblue1')
    let v:colornames['cadetblue1'] = '#98f5ff'
endif
if !has_key(v:colornames, 'cadetblue2')
    let v:colornames['cadetblue2'] = '#8ee5ee'
endif
if !has_key(v:colornames, 'cadetblue3')
    let v:colornames['cadetblue3'] = '#7ac5cd'
endif
if !has_key(v:colornames, 'cadetblue4')
    let v:colornames['cadetblue4'] = '#53868b'
endif
if !has_key(v:colornames, 'turquoise1')
    let v:colornames['turquoise1'] = '#00f5ff'
endif
if !has_key(v:colornames, 'turquoise2')
    let v:colornames['turquoise2'] = '#00e5ee'
endif
if !has_key(v:colornames, 'turquoise3')
    let v:colornames['turquoise3'] = '#00c5cd'
endif
if !has_key(v:colornames, 'turquoise4')
    let v:colornames['turquoise4'] = '#00868b'
endif
if !has_key(v:colornames, 'cyan1')
    let v:colornames['cyan1'] = '#00ffff'
endif
if !has_key(v:colornames, 'cyan2')
    let v:colornames['cyan2'] = '#00eeee'
endif
if !has_key(v:colornames, 'cyan3')
    let v:colornames['cyan3'] = '#00cdcd'
endif
if !has_key(v:colornames, 'cyan4')
    let v:colornames['cyan4'] = '#008b8b'
endif
if !has_key(v:colornames, 'darkslategray1')
    let v:colornames['darkslategray1'] = '#97ffff'
endif
if !has_key(v:colornames, 'darkslategray2')
    let v:colornames['darkslategray2'] = '#8deeee'
endif
if !has_key(v:colornames, 'darkslategray3')
    let v:colornames['darkslategray3'] = '#79cdcd'
endif
if !has_key(v:colornames, 'darkslategray4')
    let v:colornames['darkslategray4'] = '#528b8b'
endif
if !has_key(v:colornames, 'aquamarine1')
    let v:colornames['aquamarine1'] = '#7fffd4'
endif
if !has_key(v:colornames, 'aquamarine2')
    let v:colornames['aquamarine2'] = '#76eec6'
endif
if !has_key(v:colornames, 'aquamarine3')
    let v:colornames['aquamarine3'] = '#66cdaa'
endif
if !has_key(v:colornames, 'aquamarine4')
    let v:colornames['aquamarine4'] = '#458b74'
endif
if !has_key(v:colornames, 'darkseagreen1')
    let v:colornames['darkseagreen1'] = '#c1ffc1'
endif
if !has_key(v:colornames, 'darkseagreen2')
    let v:colornames['darkseagreen2'] = '#b4eeb4'
endif
if !has_key(v:colornames, 'darkseagreen3')
    let v:colornames['darkseagreen3'] = '#9bcd9b'
endif
if !has_key(v:colornames, 'darkseagreen4')
    let v:colornames['darkseagreen4'] = '#698b69'
endif
if !has_key(v:colornames, 'seagreen1')
    let v:colornames['seagreen1'] = '#54ff9f'
endif
if !has_key(v:colornames, 'seagreen2')
    let v:colornames['seagreen2'] = '#4eee94'
endif
if !has_key(v:colornames, 'seagreen3')
    let v:colornames['seagreen3'] = '#43cd80'
endif
if !has_key(v:colornames, 'seagreen4')
    let v:colornames['seagreen4'] = '#2e8b57'
endif
if !has_key(v:colornames, 'palegreen1')
    let v:colornames['palegreen1'] = '#9aff9a'
endif
if !has_key(v:colornames, 'palegreen2')
    let v:colornames['palegreen2'] = '#90ee90'
endif
if !has_key(v:colornames, 'palegreen3')
    let v:colornames['palegreen3'] = '#7ccd7c'
endif
if !has_key(v:colornames, 'palegreen4')
    let v:colornames['palegreen4'] = '#548b54'
endif
if !has_key(v:colornames, 'springgreen1')
    let v:colornames['springgreen1'] = '#00ff7f'
endif
if !has_key(v:colornames, 'springgreen2')
    let v:colornames['springgreen2'] = '#00ee76'
endif
if !has_key(v:colornames, 'springgreen3')
    let v:colornames['springgreen3'] = '#00cd66'
endif
if !has_key(v:colornames, 'springgreen4')
    let v:colornames['springgreen4'] = '#008b45'
endif
if !has_key(v:colornames, 'green1')
    let v:colornames['green1'] = '#00ff00'
endif
if !has_key(v:colornames, 'green2')
    let v:colornames['green2'] = '#00ee00'
endif
if !has_key(v:colornames, 'green3')
    let v:colornames['green3'] = '#00cd00'
endif
if !has_key(v:colornames, 'green4')
    let v:colornames['green4'] = '#008b00'
endif
if !has_key(v:colornames, 'chartreuse1')
    let v:colornames['chartreuse1'] = '#7fff00'
endif
if !has_key(v:colornames, 'chartreuse2')
    let v:colornames['chartreuse2'] = '#76ee00'
endif
if !has_key(v:colornames, 'chartreuse3')
    let v:colornames['chartreuse3'] = '#66cd00'
endif
if !has_key(v:colornames, 'chartreuse4')
    let v:colornames['chartreuse4'] = '#458b00'
endif
if !has_key(v:colornames, 'olivedrab1')
    let v:colornames['olivedrab1'] = '#c0ff3e'
endif
if !has_key(v:colornames, 'olivedrab2')
    let v:colornames['olivedrab2'] = '#b3ee3a'
endif
if !has_key(v:colornames, 'olivedrab3')
    let v:colornames['olivedrab3'] = '#9acd32'
endif
if !has_key(v:colornames, 'olivedrab4')
    let v:colornames['olivedrab4'] = '#698b22'
endif
if !has_key(v:colornames, 'darkolivegreen1')
    let v:colornames['darkolivegreen1'] = '#caff70'
endif
if !has_key(v:colornames, 'darkolivegreen2')
    let v:colornames['darkolivegreen2'] = '#bcee68'
endif
if !has_key(v:colornames, 'darkolivegreen3')
    let v:colornames['darkolivegreen3'] = '#a2cd5a'
endif
if !has_key(v:colornames, 'darkolivegreen4')
    let v:colornames['darkolivegreen4'] = '#6e8b3d'
endif
if !has_key(v:colornames, 'khaki1')
    let v:colornames['khaki1'] = '#fff68f'
endif
if !has_key(v:colornames, 'khaki2')
    let v:colornames['khaki2'] = '#eee685'
endif
if !has_key(v:colornames, 'khaki3')
    let v:colornames['khaki3'] = '#cdc673'
endif
if !has_key(v:colornames, 'khaki4')
    let v:colornames['khaki4'] = '#8b864e'
endif
if !has_key(v:colornames, 'lightgoldenrod1')
    let v:colornames['lightgoldenrod1'] = '#ffec8b'
endif
if !has_key(v:colornames, 'lightgoldenrod2')
    let v:colornames['lightgoldenrod2'] = '#eedc82'
endif
if !has_key(v:colornames, 'lightgoldenrod3')
    let v:colornames['lightgoldenrod3'] = '#cdbe70'
endif
if !has_key(v:colornames, 'lightgoldenrod4')
    let v:colornames['lightgoldenrod4'] = '#8b814c'
endif
if !has_key(v:colornames, 'lightyellow1')
    let v:colornames['lightyellow1'] = '#ffffe0'
endif
if !has_key(v:colornames, 'lightyellow2')
    let v:colornames['lightyellow2'] = '#eeeed1'
endif
if !has_key(v:colornames, 'lightyellow3')
    let v:colornames['lightyellow3'] = '#cdcdb4'
endif
if !has_key(v:colornames, 'lightyellow4')
    let v:colornames['lightyellow4'] = '#8b8b7a'
endif
if !has_key(v:colornames, 'yellow1')
    let v:colornames['yellow1'] = '#ffff00'
endif
if !has_key(v:colornames, 'yellow2')
    let v:colornames['yellow2'] = '#eeee00'
endif
if !has_key(v:colornames, 'yellow3')
    let v:colornames['yellow3'] = '#cdcd00'
endif
if !has_key(v:colornames, 'yellow4')
    let v:colornames['yellow4'] = '#8b8b00'
endif
if !has_key(v:colornames, 'gold1')
    let v:colornames['gold1'] = '#ffd700'
endif
if !has_key(v:colornames, 'gold2')
    let v:colornames['gold2'] = '#eec900'
endif
if !has_key(v:colornames, 'gold3')
    let v:colornames['gold3'] = '#cdad00'
endif
if !has_key(v:colornames, 'gold4')
    let v:colornames['gold4'] = '#8b7500'
endif
if !has_key(v:colornames, 'goldenrod1')
    let v:colornames['goldenrod1'] = '#ffc125'
endif
if !has_key(v:colornames, 'goldenrod2')
    let v:colornames['goldenrod2'] = '#eeb422'
endif
if !has_key(v:colornames, 'goldenrod3')
    let v:colornames['goldenrod3'] = '#cd9b1d'
endif
if !has_key(v:colornames, 'goldenrod4')
    let v:colornames['goldenrod4'] = '#8b6914'
endif
if !has_key(v:colornames, 'darkgoldenrod1')
    let v:colornames['darkgoldenrod1'] = '#ffb90f'
endif
if !has_key(v:colornames, 'darkgoldenrod2')
    let v:colornames['darkgoldenrod2'] = '#eead0e'
endif
if !has_key(v:colornames, 'darkgoldenrod3')
    let v:colornames['darkgoldenrod3'] = '#cd950c'
endif
if !has_key(v:colornames, 'darkgoldenrod4')
    let v:colornames['darkgoldenrod4'] = '#8b6508'
endif
if !has_key(v:colornames, 'rosybrown1')
    let v:colornames['rosybrown1'] = '#ffc1c1'
endif
if !has_key(v:colornames, 'rosybrown2')
    let v:colornames['rosybrown2'] = '#eeb4b4'
endif
if !has_key(v:colornames, 'rosybrown3')
    let v:colornames['rosybrown3'] = '#cd9b9b'
endif
if !has_key(v:colornames, 'rosybrown4')
    let v:colornames['rosybrown4'] = '#8b6969'
endif
if !has_key(v:colornames, 'indianred1')
    let v:colornames['indianred1'] = '#ff6a6a'
endif
if !has_key(v:colornames, 'indianred2')
    let v:colornames['indianred2'] = '#ee6363'
endif
if !has_key(v:colornames, 'indianred3')
    let v:colornames['indianred3'] = '#cd5555'
endif
if !has_key(v:colornames, 'indianred4')
    let v:colornames['indianred4'] = '#8b3a3a'
endif
if !has_key(v:colornames, 'sienna1')
    let v:colornames['sienna1'] = '#ff8247'
endif
if !has_key(v:colornames, 'sienna2')
    let v:colornames['sienna2'] = '#ee7942'
endif
if !has_key(v:colornames, 'sienna3')
    let v:colornames['sienna3'] = '#cd6839'
endif
if !has_key(v:colornames, 'sienna4')
    let v:colornames['sienna4'] = '#8b4726'
endif
if !has_key(v:colornames, 'burlywood1')
    let v:colornames['burlywood1'] = '#ffd39b'
endif
if !has_key(v:colornames, 'burlywood2')
    let v:colornames['burlywood2'] = '#eec591'
endif
if !has_key(v:colornames, 'burlywood3')
    let v:colornames['burlywood3'] = '#cdaa7d'
endif
if !has_key(v:colornames, 'burlywood4')
    let v:colornames['burlywood4'] = '#8b7355'
endif
if !has_key(v:colornames, 'wheat1')
    let v:colornames['wheat1'] = '#ffe7ba'
endif
if !has_key(v:colornames, 'wheat2')
    let v:colornames['wheat2'] = '#eed8ae'
endif
if !has_key(v:colornames, 'wheat3')
    let v:colornames['wheat3'] = '#cdba96'
endif
if !has_key(v:colornames, 'wheat4')
    let v:colornames['wheat4'] = '#8b7e66'
endif
if !has_key(v:colornames, 'tan1')
    let v:colornames['tan1'] = '#ffa54f'
endif
if !has_key(v:colornames, 'tan2')
    let v:colornames['tan2'] = '#ee9a49'
endif
if !has_key(v:colornames, 'tan3')
    let v:colornames['tan3'] = '#cd853f'
endif
if !has_key(v:colornames, 'tan4')
    let v:colornames['tan4'] = '#8b5a2b'
endif
if !has_key(v:colornames, 'chocolate1')
    let v:colornames['chocolate1'] = '#ff7f24'
endif
if !has_key(v:colornames, 'chocolate2')
    let v:colornames['chocolate2'] = '#ee7621'
endif
if !has_key(v:colornames, 'chocolate3')
    let v:colornames['chocolate3'] = '#cd661d'
endif
if !has_key(v:colornames, 'chocolate4')
    let v:colornames['chocolate4'] = '#8b4513'
endif
if !has_key(v:colornames, 'firebrick1')
    let v:colornames['firebrick1'] = '#ff3030'
endif
if !has_key(v:colornames, 'firebrick2')
    let v:colornames['firebrick2'] = '#ee2c2c'
endif
if !has_key(v:colornames, 'firebrick3')
    let v:colornames['firebrick3'] = '#cd2626'
endif
if !has_key(v:colornames, 'firebrick4')
    let v:colornames['firebrick4'] = '#8b1a1a'
endif
if !has_key(v:colornames, 'brown1')
    let v:colornames['brown1'] = '#ff4040'
endif
if !has_key(v:colornames, 'brown2')
    let v:colornames['brown2'] = '#ee3b3b'
endif
if !has_key(v:colornames, 'brown3')
    let v:colornames['brown3'] = '#cd3333'
endif
if !has_key(v:colornames, 'brown4')
    let v:colornames['brown4'] = '#8b2323'
endif
if !has_key(v:colornames, 'salmon1')
    let v:colornames['salmon1'] = '#ff8c69'
endif
if !has_key(v:colornames, 'salmon2')
    let v:colornames['salmon2'] = '#ee8262'
endif
if !has_key(v:colornames, 'salmon3')
    let v:colornames['salmon3'] = '#cd7054'
endif
if !has_key(v:colornames, 'salmon4')
    let v:colornames['salmon4'] = '#8b4c39'
endif
if !has_key(v:colornames, 'lightsalmon1')
    let v:colornames['lightsalmon1'] = '#ffa07a'
endif
if !has_key(v:colornames, 'lightsalmon2')
    let v:colornames['lightsalmon2'] = '#ee9572'
endif
if !has_key(v:colornames, 'lightsalmon3')
    let v:colornames['lightsalmon3'] = '#cd8162'
endif
if !has_key(v:colornames, 'lightsalmon4')
    let v:colornames['lightsalmon4'] = '#8b5742'
endif
if !has_key(v:colornames, 'orange1')
    let v:colornames['orange1'] = '#ffa500'
endif
if !has_key(v:colornames, 'orange2')
    let v:colornames['orange2'] = '#ee9a00'
endif
if !has_key(v:colornames, 'orange3')
    let v:colornames['orange3'] = '#cd8500'
endif
if !has_key(v:colornames, 'orange4')
    let v:colornames['orange4'] = '#8b5a00'
endif
if !has_key(v:colornames, 'darkorange1')
    let v:colornames['darkorange1'] = '#ff7f00'
endif
if !has_key(v:colornames, 'darkorange2')
    let v:colornames['darkorange2'] = '#ee7600'
endif
if !has_key(v:colornames, 'darkorange3')
    let v:colornames['darkorange3'] = '#cd6600'
endif
if !has_key(v:colornames, 'darkorange4')
    let v:colornames['darkorange4'] = '#8b4500'
endif
if !has_key(v:colornames, 'coral1')
    let v:colornames['coral1'] = '#ff7256'
endif
if !has_key(v:colornames, 'coral2')
    let v:colornames['coral2'] = '#ee6a50'
endif
if !has_key(v:colornames, 'coral3')
    let v:colornames['coral3'] = '#cd5b45'
endif
if !has_key(v:colornames, 'coral4')
    let v:colornames['coral4'] = '#8b3e2f'
endif
if !has_key(v:colornames, 'tomato1')
    let v:colornames['tomato1'] = '#ff6347'
endif
if !has_key(v:colornames, 'tomato2')
    let v:colornames['tomato2'] = '#ee5c42'
endif
if !has_key(v:colornames, 'tomato3')
    let v:colornames['tomato3'] = '#cd4f39'
endif
if !has_key(v:colornames, 'tomato4')
    let v:colornames['tomato4'] = '#8b3626'
endif
if !has_key(v:colornames, 'orangered1')
    let v:colornames['orangered1'] = '#ff4500'
endif
if !has_key(v:colornames, 'orangered2')
    let v:colornames['orangered2'] = '#ee4000'
endif
if !has_key(v:colornames, 'orangered3')
    let v:colornames['orangered3'] = '#cd3700'
endif
if !has_key(v:colornames, 'orangered4')
    let v:colornames['orangered4'] = '#8b2500'
endif
if !has_key(v:colornames, 'red1')
    let v:colornames['red1'] = '#ff0000'
endif
if !has_key(v:colornames, 'red2')
    let v:colornames['red2'] = '#ee0000'
endif
if !has_key(v:colornames, 'red3')
    let v:colornames['red3'] = '#cd0000'
endif
if !has_key(v:colornames, 'red4')
    let v:colornames['red4'] = '#8b0000'
endif
if !has_key(v:colornames, 'deeppink1')
    let v:colornames['deeppink1'] = '#ff1493'
endif
if !has_key(v:colornames, 'deeppink2')
    let v:colornames['deeppink2'] = '#ee1289'
endif
if !has_key(v:colornames, 'deeppink3')
    let v:colornames['deeppink3'] = '#cd1076'
endif
if !has_key(v:colornames, 'deeppink4')
    let v:colornames['deeppink4'] = '#8b0a50'
endif
if !has_key(v:colornames, 'hotpink1')
    let v:colornames['hotpink1'] = '#ff6eb4'
endif
if !has_key(v:colornames, 'hotpink2')
    let v:colornames['hotpink2'] = '#ee6aa7'
endif
if !has_key(v:colornames, 'hotpink3')
    let v:colornames['hotpink3'] = '#cd6090'
endif
if !has_key(v:colornames, 'hotpink4')
    let v:colornames['hotpink4'] = '#8b3a62'
endif
if !has_key(v:colornames, 'pink1')
    let v:colornames['pink1'] = '#ffb5c5'
endif
if !has_key(v:colornames, 'pink2')
    let v:colornames['pink2'] = '#eea9b8'
endif
if !has_key(v:colornames, 'pink3')
    let v:colornames['pink3'] = '#cd919e'
endif
if !has_key(v:colornames, 'pink4')
    let v:colornames['pink4'] = '#8b636c'
endif
if !has_key(v:colornames, 'lightpink1')
    let v:colornames['lightpink1'] = '#ffaeb9'
endif
if !has_key(v:colornames, 'lightpink2')
    let v:colornames['lightpink2'] = '#eea2ad'
endif
if !has_key(v:colornames, 'lightpink3')
    let v:colornames['lightpink3'] = '#cd8c95'
endif
if !has_key(v:colornames, 'lightpink4')
    let v:colornames['lightpink4'] = '#8b5f65'
endif
if !has_key(v:colornames, 'palevioletred1')
    let v:colornames['palevioletred1'] = '#ff82ab'
endif
if !has_key(v:colornames, 'palevioletred2')
    let v:colornames['palevioletred2'] = '#ee799f'
endif
if !has_key(v:colornames, 'palevioletred3')
    let v:colornames['palevioletred3'] = '#cd6889'
endif
if !has_key(v:colornames, 'palevioletred4')
    let v:colornames['palevioletred4'] = '#8b475d'
endif
if !has_key(v:colornames, 'maroon1')
    let v:colornames['maroon1'] = '#ff34b3'
endif
if !has_key(v:colornames, 'maroon2')
    let v:colornames['maroon2'] = '#ee30a7'
endif
if !has_key(v:colornames, 'maroon3')
    let v:colornames['maroon3'] = '#cd2990'
endif
if !has_key(v:colornames, 'maroon4')
    let v:colornames['maroon4'] = '#8b1c62'
endif
if !has_key(v:colornames, 'violetred1')
    let v:colornames['violetred1'] = '#ff3e96'
endif
if !has_key(v:colornames, 'violetred2')
    let v:colornames['violetred2'] = '#ee3a8c'
endif
if !has_key(v:colornames, 'violetred3')
    let v:colornames['violetred3'] = '#cd3278'
endif
if !has_key(v:colornames, 'violetred4')
    let v:colornames['violetred4'] = '#8b2252'
endif
if !has_key(v:colornames, 'magenta1')
    let v:colornames['magenta1'] = '#ff00ff'
endif
if !has_key(v:colornames, 'magenta2')
    let v:colornames['magenta2'] = '#ee00ee'
endif
if !has_key(v:colornames, 'magenta3')
    let v:colornames['magenta3'] = '#cd00cd'
endif
if !has_key(v:colornames, 'magenta4')
    let v:colornames['magenta4'] = '#8b008b'
endif
if !has_key(v:colornames, 'orchid1')
    let v:colornames['orchid1'] = '#ff83fa'
endif
if !has_key(v:colornames, 'orchid2')
    let v:colornames['orchid2'] = '#ee7ae9'
endif
if !has_key(v:colornames, 'orchid3')
    let v:colornames['orchid3'] = '#cd69c9'
endif
if !has_key(v:colornames, 'orchid4')
    let v:colornames['orchid4'] = '#8b4789'
endif
if !has_key(v:colornames, 'plum1')
    let v:colornames['plum1'] = '#ffbbff'
endif
if !has_key(v:colornames, 'plum2')
    let v:colornames['plum2'] = '#eeaeee'
endif
if !has_key(v:colornames, 'plum3')
    let v:colornames['plum3'] = '#cd96cd'
endif
if !has_key(v:colornames, 'plum4')
    let v:colornames['plum4'] = '#8b668b'
endif
if !has_key(v:colornames, 'mediumorchid1')
    let v:colornames['mediumorchid1'] = '#e066ff'
endif
if !has_key(v:colornames, 'mediumorchid2')
    let v:colornames['mediumorchid2'] = '#d15fee'
endif
if !has_key(v:colornames, 'mediumorchid3')
    let v:colornames['mediumorchid3'] = '#b452cd'
endif
if !has_key(v:colornames, 'mediumorchid4')
    let v:colornames['mediumorchid4'] = '#7a378b'
endif
if !has_key(v:colornames, 'darkorchid1')
    let v:colornames['darkorchid1'] = '#bf3eff'
endif
if !has_key(v:colornames, 'darkorchid2')
    let v:colornames['darkorchid2'] = '#b23aee'
endif
if !has_key(v:colornames, 'darkorchid3')
    let v:colornames['darkorchid3'] = '#9a32cd'
endif
if !has_key(v:colornames, 'darkorchid4')
    let v:colornames['darkorchid4'] = '#68228b'
endif
if !has_key(v:colornames, 'purple1')
    let v:colornames['purple1'] = '#9b30ff'
endif
if !has_key(v:colornames, 'purple2')
    let v:colornames['purple2'] = '#912cee'
endif
if !has_key(v:colornames, 'purple3')
    let v:colornames['purple3'] = '#7d26cd'
endif
if !has_key(v:colornames, 'purple4')
    let v:colornames['purple4'] = '#551a8b'
endif
if !has_key(v:colornames, 'mediumpurple1')
    let v:colornames['mediumpurple1'] = '#ab82ff'
endif
if !has_key(v:colornames, 'mediumpurple2')
    let v:colornames['mediumpurple2'] = '#9f79ee'
endif
if !has_key(v:colornames, 'mediumpurple3')
    let v:colornames['mediumpurple3'] = '#8968cd'
endif
if !has_key(v:colornames, 'mediumpurple4')
    let v:colornames['mediumpurple4'] = '#5d478b'
endif
if !has_key(v:colornames, 'thistle1')
    let v:colornames['thistle1'] = '#ffe1ff'
endif
if !has_key(v:colornames, 'thistle2')
    let v:colornames['thistle2'] = '#eed2ee'
endif
if !has_key(v:colornames, 'thistle3')
    let v:colornames['thistle3'] = '#cdb5cd'
endif
if !has_key(v:colornames, 'thistle4')
    let v:colornames['thistle4'] = '#8b7b8b'
endif
if !has_key(v:colornames, 'gray0')
    let v:colornames['gray0'] = '#000000'
endif
if !has_key(v:colornames, 'grey0')
    let v:colornames['grey0'] = '#000000'
endif
if !has_key(v:colornames, 'gray1')
    let v:colornames['gray1'] = '#030303'
endif
if !has_key(v:colornames, 'grey1')
    let v:colornames['grey1'] = '#030303'
endif
if !has_key(v:colornames, 'gray2')
    let v:colornames['gray2'] = '#050505'
endif
if !has_key(v:colornames, 'grey2')
    let v:colornames['grey2'] = '#050505'
endif
if !has_key(v:colornames, 'gray3')
    let v:colornames['gray3'] = '#080808'
endif
if !has_key(v:colornames, 'grey3')
    let v:colornames['grey3'] = '#080808'
endif
if !has_key(v:colornames, 'gray4')
    let v:colornames['gray4'] = '#0a0a0a'
endif
if !has_key(v:colornames, 'grey4')
    let v:colornames['grey4'] = '#0a0a0a'
endif
if !has_key(v:colornames, 'gray5')
    let v:colornames['gray5'] = '#0d0d0d'
endif
if !has_key(v:colornames, 'grey5')
    let v:colornames['grey5'] = '#0d0d0d'
endif
if !has_key(v:colornames, 'gray6')
    let v:colornames['gray6'] = '#0f0f0f'
endif
if !has_key(v:colornames, 'grey6')
    let v:colornames['grey6'] = '#0f0f0f'
endif
if !has_key(v:colornames, 'gray7')
    let v:colornames['gray7'] = '#121212'
endif
if !has_key(v:colornames, 'grey7')
    let v:colornames['grey7'] = '#121212'
endif
if !has_key(v:colornames, 'gray8')
    let v:colornames['gray8'] = '#141414'
endif
if !has_key(v:colornames, 'grey8')
    let v:colornames['grey8'] = '#141414'
endif
if !has_key(v:colornames, 'gray9')
    let v:colornames['gray9'] = '#171717'
endif
if !has_key(v:colornames, 'grey9')
    let v:colornames['grey9'] = '#171717'
endif
if !has_key(v:colornames, 'gray10')
    let v:colornames['gray10'] = '#1a1a1a'
endif
if !has_key(v:colornames, 'grey10')
    let v:colornames['grey10'] = '#1a1a1a'
endif
if !has_key(v:colornames, 'gray11')
    let v:colornames['gray11'] = '#1c1c1c'
endif
if !has_key(v:colornames, 'grey11')
    let v:colornames['grey11'] = '#1c1c1c'
endif
if !has_key(v:colornames, 'gray12')
    let v:colornames['gray12'] = '#1f1f1f'
endif
if !has_key(v:colornames, 'grey12')
    let v:colornames['grey12'] = '#1f1f1f'
endif
if !has_key(v:colornames, 'gray13')
    let v:colornames['gray13'] = '#212121'
endif
if !has_key(v:colornames, 'grey13')
    let v:colornames['grey13'] = '#212121'
endif
if !has_key(v:colornames, 'gray14')
    let v:colornames['gray14'] = '#242424'
endif
if !has_key(v:colornames, 'grey14')
    let v:colornames['grey14'] = '#242424'
endif
if !has_key(v:colornames, 'gray15')
    let v:colornames['gray15'] = '#262626'
endif
if !has_key(v:colornames, 'grey15')
    let v:colornames['grey15'] = '#262626'
endif
if !has_key(v:colornames, 'gray16')
    let v:colornames['gray16'] = '#292929'
endif
if !has_key(v:colornames, 'grey16')
    let v:colornames['grey16'] = '#292929'
endif
if !has_key(v:colornames, 'gray17')
    let v:colornames['gray17'] = '#2b2b2b'
endif
if !has_key(v:colornames, 'grey17')
    let v:colornames['grey17'] = '#2b2b2b'
endif
if !has_key(v:colornames, 'gray18')
    let v:colornames['gray18'] = '#2e2e2e'
endif
if !has_key(v:colornames, 'grey18')
    let v:colornames['grey18'] = '#2e2e2e'
endif
if !has_key(v:colornames, 'gray19')
    let v:colornames['gray19'] = '#303030'
endif
if !has_key(v:colornames, 'grey19')
    let v:colornames['grey19'] = '#303030'
endif
if !has_key(v:colornames, 'gray20')
    let v:colornames['gray20'] = '#333333'
endif
if !has_key(v:colornames, 'grey20')
    let v:colornames['grey20'] = '#333333'
endif
if !has_key(v:colornames, 'gray21')
    let v:colornames['gray21'] = '#363636'
endif
if !has_key(v:colornames, 'grey21')
    let v:colornames['grey21'] = '#363636'
endif
if !has_key(v:colornames, 'gray22')
    let v:colornames['gray22'] = '#383838'
endif
if !has_key(v:colornames, 'grey22')
    let v:colornames['grey22'] = '#383838'
endif
if !has_key(v:colornames, 'gray23')
    let v:colornames['gray23'] = '#3b3b3b'
endif
if !has_key(v:colornames, 'grey23')
    let v:colornames['grey23'] = '#3b3b3b'
endif
if !has_key(v:colornames, 'gray24')
    let v:colornames['gray24'] = '#3d3d3d'
endif
if !has_key(v:colornames, 'grey24')
    let v:colornames['grey24'] = '#3d3d3d'
endif
if !has_key(v:colornames, 'gray25')
    let v:colornames['gray25'] = '#404040'
endif
if !has_key(v:colornames, 'grey25')
    let v:colornames['grey25'] = '#404040'
endif
if !has_key(v:colornames, 'gray26')
    let v:colornames['gray26'] = '#424242'
endif
if !has_key(v:colornames, 'grey26')
    let v:colornames['grey26'] = '#424242'
endif
if !has_key(v:colornames, 'gray27')
    let v:colornames['gray27'] = '#454545'
endif
if !has_key(v:colornames, 'grey27')
    let v:colornames['grey27'] = '#454545'
endif
if !has_key(v:colornames, 'gray28')
    let v:colornames['gray28'] = '#474747'
endif
if !has_key(v:colornames, 'grey28')
    let v:colornames['grey28'] = '#474747'
endif
if !has_key(v:colornames, 'gray29')
    let v:colornames['gray29'] = '#4a4a4a'
endif
if !has_key(v:colornames, 'grey29')
    let v:colornames['grey29'] = '#4a4a4a'
endif
if !has_key(v:colornames, 'gray30')
    let v:colornames['gray30'] = '#4d4d4d'
endif
if !has_key(v:colornames, 'grey30')
    let v:colornames['grey30'] = '#4d4d4d'
endif
if !has_key(v:colornames, 'gray31')
    let v:colornames['gray31'] = '#4f4f4f'
endif
if !has_key(v:colornames, 'grey31')
    let v:colornames['grey31'] = '#4f4f4f'
endif
if !has_key(v:colornames, 'gray32')
    let v:colornames['gray32'] = '#525252'
endif
if !has_key(v:colornames, 'grey32')
    let v:colornames['grey32'] = '#525252'
endif
if !has_key(v:colornames, 'gray33')
    let v:colornames['gray33'] = '#545454'
endif
if !has_key(v:colornames, 'grey33')
    let v:colornames['grey33'] = '#545454'
endif
if !has_key(v:colornames, 'gray34')
    let v:colornames['gray34'] = '#575757'
endif
if !has_key(v:colornames, 'grey34')
    let v:colornames['grey34'] = '#575757'
endif
if !has_key(v:colornames, 'gray35')
    let v:colornames['gray35'] = '#595959'
endif
if !has_key(v:colornames, 'grey35')
    let v:colornames['grey35'] = '#595959'
endif
if !has_key(v:colornames, 'gray36')
    let v:colornames['gray36'] = '#5c5c5c'
endif
if !has_key(v:colornames, 'grey36')
    let v:colornames['grey36'] = '#5c5c5c'
endif
if !has_key(v:colornames, 'gray37')
    let v:colornames['gray37'] = '#5e5e5e'
endif
if !has_key(v:colornames, 'grey37')
    let v:colornames['grey37'] = '#5e5e5e'
endif
if !has_key(v:colornames, 'gray38')
    let v:colornames['gray38'] = '#616161'
endif
if !has_key(v:colornames, 'grey38')
    let v:colornames['grey38'] = '#616161'
endif
if !has_key(v:colornames, 'gray39')
    let v:colornames['gray39'] = '#636363'
endif
if !has_key(v:colornames, 'grey39')
    let v:colornames['grey39'] = '#636363'
endif
if !has_key(v:colornames, 'gray40')
    let v:colornames['gray40'] = '#666666'
endif
if !has_key(v:colornames, 'grey40')
    let v:colornames['grey40'] = '#666666'
endif
if !has_key(v:colornames, 'gray41')
    let v:colornames['gray41'] = '#696969'
endif
if !has_key(v:colornames, 'grey41')
    let v:colornames['grey41'] = '#696969'
endif
if !has_key(v:colornames, 'gray42')
    let v:colornames['gray42'] = '#6b6b6b'
endif
if !has_key(v:colornames, 'grey42')
    let v:colornames['grey42'] = '#6b6b6b'
endif
if !has_key(v:colornames, 'gray43')
    let v:colornames['gray43'] = '#6e6e6e'
endif
if !has_key(v:colornames, 'grey43')
    let v:colornames['grey43'] = '#6e6e6e'
endif
if !has_key(v:colornames, 'gray44')
    let v:colornames['gray44'] = '#707070'
endif
if !has_key(v:colornames, 'grey44')
    let v:colornames['grey44'] = '#707070'
endif
if !has_key(v:colornames, 'gray45')
    let v:colornames['gray45'] = '#737373'
endif
if !has_key(v:colornames, 'grey45')
    let v:colornames['grey45'] = '#737373'
endif
if !has_key(v:colornames, 'gray46')
    let v:colornames['gray46'] = '#757575'
endif
if !has_key(v:colornames, 'grey46')
    let v:colornames['grey46'] = '#757575'
endif
if !has_key(v:colornames, 'gray47')
    let v:colornames['gray47'] = '#787878'
endif
if !has_key(v:colornames, 'grey47')
    let v:colornames['grey47'] = '#787878'
endif
if !has_key(v:colornames, 'gray48')
    let v:colornames['gray48'] = '#7a7a7a'
endif
if !has_key(v:colornames, 'grey48')
    let v:colornames['grey48'] = '#7a7a7a'
endif
if !has_key(v:colornames, 'gray49')
    let v:colornames['gray49'] = '#7d7d7d'
endif
if !has_key(v:colornames, 'grey49')
    let v:colornames['grey49'] = '#7d7d7d'
endif
if !has_key(v:colornames, 'gray50')
    let v:colornames['gray50'] = '#7f7f7f'
endif
if !has_key(v:colornames, 'grey50')
    let v:colornames['grey50'] = '#7f7f7f'
endif
if !has_key(v:colornames, 'gray51')
    let v:colornames['gray51'] = '#828282'
endif
if !has_key(v:colornames, 'grey51')
    let v:colornames['grey51'] = '#828282'
endif
if !has_key(v:colornames, 'gray52')
    let v:colornames['gray52'] = '#858585'
endif
if !has_key(v:colornames, 'grey52')
    let v:colornames['grey52'] = '#858585'
endif
if !has_key(v:colornames, 'gray53')
    let v:colornames['gray53'] = '#878787'
endif
if !has_key(v:colornames, 'grey53')
    let v:colornames['grey53'] = '#878787'
endif
if !has_key(v:colornames, 'gray54')
    let v:colornames['gray54'] = '#8a8a8a'
endif
if !has_key(v:colornames, 'grey54')
    let v:colornames['grey54'] = '#8a8a8a'
endif
if !has_key(v:colornames, 'gray55')
    let v:colornames['gray55'] = '#8c8c8c'
endif
if !has_key(v:colornames, 'grey55')
    let v:colornames['grey55'] = '#8c8c8c'
endif
if !has_key(v:colornames, 'gray56')
    let v:colornames['gray56'] = '#8f8f8f'
endif
if !has_key(v:colornames, 'grey56')
    let v:colornames['grey56'] = '#8f8f8f'
endif
if !has_key(v:colornames, 'gray57')
    let v:colornames['gray57'] = '#919191'
endif
if !has_key(v:colornames, 'grey57')
    let v:colornames['grey57'] = '#919191'
endif
if !has_key(v:colornames, 'gray58')
    let v:colornames['gray58'] = '#949494'
endif
if !has_key(v:colornames, 'grey58')
    let v:colornames['grey58'] = '#949494'
endif
if !has_key(v:colornames, 'gray59')
    let v:colornames['gray59'] = '#969696'
endif
if !has_key(v:colornames, 'grey59')
    let v:colornames['grey59'] = '#969696'
endif
if !has_key(v:colornames, 'gray60')
    let v:colornames['gray60'] = '#999999'
endif
if !has_key(v:colornames, 'grey60')
    let v:colornames['grey60'] = '#999999'
endif
if !has_key(v:colornames, 'gray61')
    let v:colornames['gray61'] = '#9c9c9c'
endif
if !has_key(v:colornames, 'grey61')
    let v:colornames['grey61'] = '#9c9c9c'
endif
if !has_key(v:colornames, 'gray62')
    let v:colornames['gray62'] = '#9e9e9e'
endif
if !has_key(v:colornames, 'grey62')
    let v:colornames['grey62'] = '#9e9e9e'
endif
if !has_key(v:colornames, 'gray63')
    let v:colornames['gray63'] = '#a1a1a1'
endif
if !has_key(v:colornames, 'grey63')
    let v:colornames['grey63'] = '#a1a1a1'
endif
if !has_key(v:colornames, 'gray64')
    let v:colornames['gray64'] = '#a3a3a3'
endif
if !has_key(v:colornames, 'grey64')
    let v:colornames['grey64'] = '#a3a3a3'
endif
if !has_key(v:colornames, 'gray65')
    let v:colornames['gray65'] = '#a6a6a6'
endif
if !has_key(v:colornames, 'grey65')
    let v:colornames['grey65'] = '#a6a6a6'
endif
if !has_key(v:colornames, 'gray66')
    let v:colornames['gray66'] = '#a8a8a8'
endif
if !has_key(v:colornames, 'grey66')
    let v:colornames['grey66'] = '#a8a8a8'
endif
if !has_key(v:colornames, 'gray67')
    let v:colornames['gray67'] = '#ababab'
endif
if !has_key(v:colornames, 'grey67')
    let v:colornames['grey67'] = '#ababab'
endif
if !has_key(v:colornames, 'gray68')
    let v:colornames['gray68'] = '#adadad'
endif
if !has_key(v:colornames, 'grey68')
    let v:colornames['grey68'] = '#adadad'
endif
if !has_key(v:colornames, 'gray69')
    let v:colornames['gray69'] = '#b0b0b0'
endif
if !has_key(v:colornames, 'grey69')
    let v:colornames['grey69'] = '#b0b0b0'
endif
if !has_key(v:colornames, 'gray70')
    let v:colornames['gray70'] = '#b3b3b3'
endif
if !has_key(v:colornames, 'grey70')
    let v:colornames['grey70'] = '#b3b3b3'
endif
if !has_key(v:colornames, 'gray71')
    let v:colornames['gray71'] = '#b5b5b5'
endif
if !has_key(v:colornames, 'grey71')
    let v:colornames['grey71'] = '#b5b5b5'
endif
if !has_key(v:colornames, 'gray72')
    let v:colornames['gray72'] = '#b8b8b8'
endif
if !has_key(v:colornames, 'grey72')
    let v:colornames['grey72'] = '#b8b8b8'
endif
if !has_key(v:colornames, 'gray73')
    let v:colornames['gray73'] = '#bababa'
endif
if !has_key(v:colornames, 'grey73')
    let v:colornames['grey73'] = '#bababa'
endif
if !has_key(v:colornames, 'gray74')
    let v:colornames['gray74'] = '#bdbdbd'
endif
if !has_key(v:colornames, 'grey74')
    let v:colornames['grey74'] = '#bdbdbd'
endif
if !has_key(v:colornames, 'gray75')
    let v:colornames['gray75'] = '#bfbfbf'
endif
if !has_key(v:colornames, 'grey75')
    let v:colornames['grey75'] = '#bfbfbf'
endif
if !has_key(v:colornames, 'gray76')
    let v:colornames['gray76'] = '#c2c2c2'
endif
if !has_key(v:colornames, 'grey76')
    let v:colornames['grey76'] = '#c2c2c2'
endif
if !has_key(v:colornames, 'gray77')
    let v:colornames['gray77'] = '#c4c4c4'
endif
if !has_key(v:colornames, 'grey77')
    let v:colornames['grey77'] = '#c4c4c4'
endif
if !has_key(v:colornames, 'gray78')
    let v:colornames['gray78'] = '#c7c7c7'
endif
if !has_key(v:colornames, 'grey78')
    let v:colornames['grey78'] = '#c7c7c7'
endif
if !has_key(v:colornames, 'gray79')
    let v:colornames['gray79'] = '#c9c9c9'
endif
if !has_key(v:colornames, 'grey79')
    let v:colornames['grey79'] = '#c9c9c9'
endif
if !has_key(v:colornames, 'gray80')
    let v:colornames['gray80'] = '#cccccc'
endif
if !has_key(v:colornames, 'grey80')
    let v:colornames['grey80'] = '#cccccc'
endif
if !has_key(v:colornames, 'gray81')
    let v:colornames['gray81'] = '#cfcfcf'
endif
if !has_key(v:colornames, 'grey81')
    let v:colornames['grey81'] = '#cfcfcf'
endif
if !has_key(v:colornames, 'gray82')
    let v:colornames['gray82'] = '#d1d1d1'
endif
if !has_key(v:colornames, 'grey82')
    let v:colornames['grey82'] = '#d1d1d1'
endif
if !has_key(v:colornames, 'gray83')
    let v:colornames['gray83'] = '#d4d4d4'
endif
if !has_key(v:colornames, 'grey83')
    let v:colornames['grey83'] = '#d4d4d4'
endif
if !has_key(v:colornames, 'gray84')
    let v:colornames['gray84'] = '#d6d6d6'
endif
if !has_key(v:colornames, 'grey84')
    let v:colornames['grey84'] = '#d6d6d6'
endif
if !has_key(v:colornames, 'gray85')
    let v:colornames['gray85'] = '#d9d9d9'
endif
if !has_key(v:colornames, 'grey85')
    let v:colornames['grey85'] = '#d9d9d9'
endif
if !has_key(v:colornames, 'gray86')
    let v:colornames['gray86'] = '#dbdbdb'
endif
if !has_key(v:colornames, 'grey86')
    let v:colornames['grey86'] = '#dbdbdb'
endif
if !has_key(v:colornames, 'gray87')
    let v:colornames['gray87'] = '#dedede'
endif
if !has_key(v:colornames, 'grey87')
    let v:colornames['grey87'] = '#dedede'
endif
if !has_key(v:colornames, 'gray88')
    let v:colornames['gray88'] = '#e0e0e0'
endif
if !has_key(v:colornames, 'grey88')
    let v:colornames['grey88'] = '#e0e0e0'
endif
if !has_key(v:colornames, 'gray89')
    let v:colornames['gray89'] = '#e3e3e3'
endif
if !has_key(v:colornames, 'grey89')
    let v:colornames['grey89'] = '#e3e3e3'
endif
if !has_key(v:colornames, 'gray90')
    let v:colornames['gray90'] = '#e5e5e5'
endif
if !has_key(v:colornames, 'grey90')
    let v:colornames['grey90'] = '#e5e5e5'
endif
if !has_key(v:colornames, 'gray91')
    let v:colornames['gray91'] = '#e8e8e8'
endif
if !has_key(v:colornames, 'grey91')
    let v:colornames['grey91'] = '#e8e8e8'
endif
if !has_key(v:colornames, 'gray92')
    let v:colornames['gray92'] = '#ebebeb'
endif
if !has_key(v:colornames, 'grey92')
    let v:colornames['grey92'] = '#ebebeb'
endif
if !has_key(v:colornames, 'gray93')
    let v:colornames['gray93'] = '#ededed'
endif
if !has_key(v:colornames, 'grey93')
    let v:colornames['grey93'] = '#ededed'
endif
if !has_key(v:colornames, 'gray94')
    let v:colornames['gray94'] = '#f0f0f0'
endif
if !has_key(v:colornames, 'grey94')
    let v:colornames['grey94'] = '#f0f0f0'
endif
if !has_key(v:colornames, 'gray95')
    let v:colornames['gray95'] = '#f2f2f2'
endif
if !has_key(v:colornames, 'grey95')
    let v:colornames['grey95'] = '#f2f2f2'
endif
if !has_key(v:colornames, 'gray96')
    let v:colornames['gray96'] = '#f5f5f5'
endif
if !has_key(v:colornames, 'grey96')
    let v:colornames['grey96'] = '#f5f5f5'
endif
if !has_key(v:colornames, 'gray97')
    let v:colornames['gray97'] = '#f7f7f7'
endif
if !has_key(v:colornames, 'grey97')
    let v:colornames['grey97'] = '#f7f7f7'
endif
if !has_key(v:colornames, 'gray98')
    let v:colornames['gray98'] = '#fafafa'
endif
if !has_key(v:colornames, 'grey98')
    let v:colornames['grey98'] = '#fafafa'
endif
if !has_key(v:colornames, 'gray99')
    let v:colornames['gray99'] = '#fcfcfc'
endif
if !has_key(v:colornames, 'grey99')
    let v:colornames['grey99'] = '#fcfcfc'
endif
if !has_key(v:colornames, 'gray100')
    let v:colornames['gray100'] = '#ffffff'
endif
if !has_key(v:colornames, 'grey100')
    let v:colornames['grey100'] = '#ffffff'
endif
if !has_key(v:colornames, 'dark grey')
    let v:colornames['dark grey'] = '#a9a9a9'
endif
if !has_key(v:colornames, 'darkgrey')
    let v:colornames['darkgrey'] = '#a9a9a9'
endif
if !has_key(v:colornames, 'dark gray')
    let v:colornames['dark gray'] = '#a9a9a9'
endif
if !has_key(v:colornames, 'darkgray')
    let v:colornames['darkgray'] = '#a9a9a9'
endif
if !has_key(v:colornames, 'dark blue')
    let v:colornames['dark blue'] = '#00008b'
endif
if !has_key(v:colornames, 'darkblue')
    let v:colornames['darkblue'] = '#00008b'
endif
if !has_key(v:colornames, 'dark cyan')
    let v:colornames['dark cyan'] = '#008b8b'
endif
if !has_key(v:colornames, 'darkcyan')
    let v:colornames['darkcyan'] = '#008b8b'
endif
if !has_key(v:colornames, 'dark magenta')
    let v:colornames['dark magenta'] = '#8b008b'
endif
if !has_key(v:colornames, 'darkmagenta')
    let v:colornames['darkmagenta'] = '#8b008b'
endif
if !has_key(v:colornames, 'dark red')
    let v:colornames['dark red'] = '#8b0000'
endif
if !has_key(v:colornames, 'darkred')
    let v:colornames['darkred'] = '#8b0000'
endif
if !has_key(v:colornames, 'light green')
    let v:colornames['light green'] = '#90ee90'
endif
if !has_key(v:colornames, 'lightgreen')
    let v:colornames['lightgreen'] = '#90ee90'
endif
if !has_key(v:colornames, 'crimson')
    let v:colornames['crimson'] = '#dc143c'
endif
if !has_key(v:colornames, 'indigo')
    let v:colornames['indigo'] = '#4b0082'
endif
if !has_key(v:colornames, 'olive')
    let v:colornames['olive'] = '#808000'
endif
if !has_key(v:colornames, 'rebecca purple')
    let v:colornames['rebecca purple'] = '#663399'
endif
if !has_key(v:colornames, 'rebeccapurple')
    let v:colornames['rebeccapurple'] = '#663399'
endif
if !has_key(v:colornames, 'silver')
    let v:colornames['silver'] = '#c0c0c0'
endif
if !has_key(v:colornames, 'teal')
    let v:colornames['teal'] = '#008080'
endif
