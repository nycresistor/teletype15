#!/usr/bin/python
import sys

# Codes are represented as strings of signals.
# Start/stop bits are not included.
# Marking elements are indicated as "#", spacing as " ".
# Elements are in order of transmission.

# We're going to shoehorn the letter and figure shift codes
# into ascii by mapping them to the little-used device control
# codes.  Figure mode is DC1; letters, DC2.
figures = '\x11'
letters = '\x12'

CodeUSTTY_letter = {
    'a'  : "##   ",
    'b'  : "#  ##",
    'c'  : " ### ",
    'd'  : "#  # ",
    'e'  : "#    ",
    'f'  : "# ## ",
    'g'  : " # ##",
    'h'  : "  # #",
    'i'  : " ##  ",
    'j'  : "## # ",
    'k'  : "#### ",
    'l'  : " #  #",
    'm'  : "  ###",
    'n'  : "  ## ",
    'o'  : "   ##",
    'p'  : " ## #",
    'q'  : "### #",
    'r'  : " # # ",
    's'  : "# #  ",
    't'  : "    #",
    'u'  : "###  ",
    'v'  : " ####",
    'w'  : "##  #",
    'x'  : "# ###",
    'y'  : "# # #",
    'z'  : "#   #"
}

CodeUSTTY_figure = {
    '-'  : "##   ",
    '?'  : "#  ##",
    ':'  : " ### ",
    '$'  : "#  # ",
    '3'  : "#    ",
    '!'  : "# ## ",
    '&'  : " # ##",
    '#'  : "  # #",
    '8'  : " ##  ",
    '\'' : "## # ",
    '('  : "#### ",
    ')'  : " #  #",
    '*'  : "  ###",
    ','  : "  ## ",
    '9'  : "   ##",
    '0'  : " ## #",
    '1'  : "### #",
    '4'  : " # # ",
    '\a' : "# #  ",
    '5'  : "    #",
    '7'  : "###  ",
    ';'  : " ####",
    '2'  : "##  #",
    '/'  : "# ###",
    '6'  : "# # #",
    '"'  : "#   #"
}

CodeUSTTY_both = {
    '\0' : "     ",
    '\n' : " #   ",
    ' '  : "  #  ",
    '\r' : "   # ",
    '['  : "## ##",
    ']'  : "#####",
    figures : "## ##",
    letters : "#####"
}

CodeUSTTY = [ CodeUSTTY_letter, CodeUSTTY_figure, CodeUSTTY_both ]

# This script produces a C file with a compact binary representation
# that maps ascii to a 5-bit code.  Each byte is arranged thus:
#
#   MSB                                       LSB
# +-----+-----+-----+-----+-----+-----+-----+-----+
# | NUL | FIG | LET | IM5 | IM4 | IM3 | IM2 | IM1 |
# +-----+-----+-----+-----+-----+-----+-----+-----+
#
# A 1 in NUL indicates that there is no mapping for the given ascii
# code.  A 1 in FIG or LET indicates a character is available in the
# figure or letter mode; some characters may be available in both.
# IM1-5 are impulses 1 through 5, in order of transmission.

def encodeChar( imStr, figure=0, letter=0 ):
    encoding = 0
    for im in imStr[::-1]:  # take imstr backwards
        encoding = encoding << 1
        if im == '#':
            encoding = encoding + 1
    if figure != 0:
        encoding = encoding + 0x40
    if letter != 0:
        encoding = encoding + 0x20
    return encoding

def encodeMap( asciiMap, imMap, figure=0, letter=0 ):
    for ( c, imRep ) in imMap.items():
        asciiMap[ ord(c) ] = encodeChar( imRep, figure, letter )

def createTable( codes ):
    [ codes_letter, codes_figure, codes_both ] = codes
    asciiMap = [0x80] * 128
    encodeMap( asciiMap, codes_letter, letter=1 )
    encodeMap( asciiMap, codes_figure, figure=1 )
    encodeMap( asciiMap, codes_both, figure=1, letter=1 )
    return asciiMap

def writeTable( outFile, prefix, codeMaps ):
    WIDTH = 8
    asciiMap = createTable(codeMaps)
    outFile.write("#include <avr/pgmspace.h>\n\n")
    outFile.write("prog_uchar %s_map[128] = {\n" % prefix)
    lines = []
    for idx in range(0,128,WIDTH):
        lines.append( ", ".join(map(hex,asciiMap[idx:idx+WIDTH])) )
    outFile.write( ",\n".join(lines) )
    outFile.write("\n};\n")

def writeHeader( outFile, path, prefix ):
    guard = "__%s__" % path.upper().replace('.','_')
    outFile.write("#ifndef  %s\n" % guard)
    outFile.write("#define  %s\n" % guard)
    outFile.write("\n")
    outFile.write("#include <avr/pgmspace.h>\n\n")
    outFile.write("#define FIGURES_CODE 0x11\n")
    outFile.write("#define LETTERS_CODE 0x12\n")
    outFile.write("extern prog_uchar %s_map[];\n" % prefix)
    outFile.write("\n")
    outFile.write("#endif /* %s */\n" % guard)

def genFiles( prefix, maps ):
    hPath = "asciiTo%s.h" % prefix.capitalize()
    tPath = "asciiTo%s.c" % prefix.capitalize()
    hFile = open(hPath,"w")
    tFile = open(tPath,"w")
    writeHeader(hFile,hPath,prefix)
    writeTable(tFile,prefix,maps)
    hFile.close()
    tFile.close()

if __name__  == "__main__":
    genFiles( "ustty", CodeUSTTY )
