'use strict' ;

let themes = {
  "dna": {
  	"default": {
    	"bases": {
      	"colour": {
        	'-': 'lightgrey',
            '=': '#333618',
            'a': '#02AAE9',
            't': '#1A356C',
            'c': '#941504',
            'g': '#F44B1A',
            'n': '#555555',
            'A': '#02AAE9',
            'T': '#1A356C',
            'C': '#941504',
            'G': '#F44B1A',
            'N': '#555555',
            // ASCII values, for Int8Array version
            45: 'lightgrey',  // '-'
            61: '#333618',    // '=' for match
            97: '#02AAE9',    // 'a'
            116: '#1A356C',   // 't'
            99: '#941504',    // 'c'
            103: '#F44B1A',   // 'g'
            110: '#555555',   // 'n'
            65: '#02AAE9',    // 'A'
            84: '#1A356C',    // 'T'
            67: '#941504',    // 'C'
            71: '#F44B1A',    // 'G'
            78: '#555555'     // 'N'
        },
        "geom": {
            '-': { "offset": -5, "height": 2 },
            '=': { "offset": -8, "height": 8 },
            'a': { "offset": -8, "height": 8 },
            't': { "offset": -8, "height": 8 },
            'c': { "offset": -8, "height": 8 },
            'g': { "offset": -8, "height": 8 },
            'n': { "offset": -6, "height": 6 },
            'A': { "offset": -8, "height": 8 },
            'T': { "offset": -8, "height": 8 },
            'C': { "offset": -8, "height": 8 },
            'G': { "offset": -8, "height": 8 },
            'N': { "offset": -6, "height": 6 },
            45: { "offset": -5, "height": 2 }, // '-'
            61: { "offset": -8, "height": 8 }, // 'm'
             97: { "offset": -8, "height": 8 }, // 'a'
            116: { "offset": -8, "height": 8 }, // 't'
             99: { "offset": -8, "height": 8 }, // 'c'
            103: { "offset": -8, "height": 8 }, // 'g'
            110: { "offset": -6, "height": 6 }, // 'n'
             65: { "offset": -8, "height": 8 }, // 'A'
             84: { "offset": -8, "height": 8 }, // 'T'
             67: { "offset": -8, "height": 8 }, // 'C'
             71: { "offset": -8, "height": 8 }, // 'G'
             78: { "offset": -6, "height": 6 }, // 'N'
        }
      }
    },
    colour: {
        "text": "#eeeeee",
        "highlight": "#FF4444",
        "background": '#222222'
    },
    displayText: {
         97: "A",
		 99: "C",
		103: "G",
		116: "T",
		 45: "", // '-' becomes empty string
		 61: "", // '=' becomes empty string
    }
  },

  "aa": {
  	"default": {
    	"bases": {
	      "colour": {
            '-': 'lightgrey',
            '=': '#333618',
            'a': '#02AAE9', // Alanine (A)
            'c': '#941504', // Cysteine (C)
            'd': '#D84C42', // Aspartic acid (D)
            'e': '#D84C93', // Glutamic acid (E)
            'f': '#6A8A92', // Phenylalanine (F)
            'g': '#F44B1A', // Glycine (G)
            'h': '#B86A6A', // Histidine (H)
            'i': '#7B9B59', // Isoleucine (I)
            'k': '#F2E400', // Lysine (K)
            'l': '#A3D64E', // Leucine (L)
            'm': '#00C8C8', // Methionine (M)
            'n': '#F16A00', // Asparagine (N)
            'p': '#6A94FF', // Proline (P)
            'q': '#F6A1D5', // Glutamine (Q)
            'r': '#8A5A9D', // Arginine (R)
            's': '#8FD88F', // Serine (S)
            't': '#1A356C', // Threonine (T)
            'v': '#62D8A2', // Valine (V)
            'w': '#96D88A', // Tryptophan (W)
            'y': '#F1C437', // Tyrosine (Y)
            'A': '#02AAE9', // Alanine (A)
            'C': '#941504', // Cysteine (C)
            'D': '#D84C42', // Aspartic acid (D)
            'E': '#D84C93', // Glutamic acid (E)
            'F': '#6A8A92', // Phenylalanine (F)
            'G': '#F44B1A', // Glycine (G)
            'H': '#B86A6A', // Histidine (H)
            'I': '#7B9B59', // Isoleucine (I)
            'K': '#F2E400', // Lysine (K)
            'L': '#A3D64E', // Leucine (L)
            'M': '#00C8C8', // Methionine (M)
            'N': '#F16A00', // Asparagine (N)
            'P': '#6A94FF', // Proline (P)
            'Q': '#F6A1D5', // Glutamine (Q)
            'R': '#8A5A9D', // Arginine (R)
            'S': '#8FD88F', // Serine (S)
            'T': '#1A356C', // Threonine (T)
            'V': '#62D8A2', // Valine (V)
            'W': '#96D88A', // Tryptophan (W)
            'Y': '#F1C437', // Tyrosine (Y)
            // ASCII values, for Int8Array version
            45: 'lightgrey',  // '-'
            61: '#333618',   // '=' for match
            97: '#02AAE9', // Alanine (A)
            99: '#941504', // Cysteine (C)
            100: '#D84C42', // Aspartic acid (D)
            101: '#D84C93', // Glutamic acid (E)
            102: '#6A8A92', // Phenylalanine (F)
            103: '#F44B1A', // Glycine (G)
            104: '#B86A6A', // Histidine (H)
            105: '#7B9B59', // Isoleucine (I)
            107: '#F2E400', // Lysine (K)
            108: '#A3D64E', // Leucine (L)
            109: '#00C8C8', // Methionine (M)
            110: '#F16A00', // Asparagine (N)
            112: '#6A94FF', // Proline (P)
            113: '#F6A1D5', // Glutamine (Q)
            114: '#8A5A9D', // Arginine (R)
            115: '#8FD88F', // Serine (S)
            116: '#1A356C', // Threonine (T)
            118: '#62D8A2', // Valine (V)
            119: '#96D88A', // Tryptophan (W)
            121: '#F1C437', // Tyrosine (Y)
            65: '#02AAE9', // Alanine (A)
            67: '#941504', // Cysteine (C)
            68: '#D84C42', // Aspartic acid (D)
            69: '#D84C93', // Glutamic acid (E)
            70: '#6A8A92', // Phenylalanine (F)
            71: '#F44B1A', // Glycine (G)
            72: '#B86A6A', // Histidine (H)
            73: '#7B9B59', // Isoleucine (I)
            75: '#F2E400', // Lysine (K)
            76: '#A3D64E', // Leucine (L)
            77: '#00C8C8', // Methionine (M)
            78: '#F16A00', // Asparagine (N)
            80: '#6A94FF', // Proline (P)
            81: '#F6A1D5', // Glutamine (Q)
            82: '#8A5A9D', // Arginine (R)
            83: '#8FD88F', // Serine (S)
            84: '#1A356C', // Threonine (T)
            86: '#62D8A2', // Valine (V)
            87: '#96D88A', // Tryptophan (W)
            89: '#F1C437' // Tyrosine (Y)
        },
        geom: {
            '-': { "offset": -5, "height": 2 },
            '=': { "offset": -8, "height": 8 },
            'a': { "offset": -8, "height": 8 }, // Alanine (A)
            'c': { "offset": -8, "height": 8 }, // Cysteine (C)
            'd': { "offset": -8, "height": 8 }, // Aspartic acid (D)
            'e': { "offset": -8, "height": 8 }, // Glutamic acid (E)
            'f': { "offset": -8, "height": 8 }, // Phenylalanine (F)
            'g': { "offset": -8, "height": 8 }, // Glycine (G)
            'h': { "offset": -8, "height": 8 }, // Histidine (H)
            'i': { "offset": -8, "height": 8 }, // Isoleucine (I)
            'k': { "offset": -8, "height": 8 }, // Lysine (K)
            'l': { "offset": -8, "height": 8 }, // Leucine (L)
            'm': { "offset": -8, "height": 8 }, // Methionine (M)
            'n': { "offset": -8, "height": 8 }, // Asparagine (N)
            'p': { "offset": -8, "height": 8 }, // Proline (P)
            'q': { "offset": -8, "height": 8 }, // Glutamine (Q)
            'r': { "offset": -8, "height": 8 }, // Arginine (R)
            's': { "offset": -8, "height": 8 }, // Serine (S)
            't': { "offset": -8, "height": 8 }, // Threonine (T)
            'v': { "offset": -8, "height": 8 }, // Valine (V)
            'w': { "offset": -8, "height": 8 }, // Tryptophan (W)
            'y': { "offset": -8, "height": 8 }, // Tyrosine (Y)
            'A': { "offset": -8, "height": 8 }, // Alanine (A)
            'C': { "offset": -8, "height": 8 }, // Cysteine (C)
            'D': { "offset": -8, "height": 8 }, // Aspartic acid (D)
            'E': { "offset": -8, "height": 8 }, // Glutamic acid (E)
            'F': { "offset": -8, "height": 8 }, // Phenylalanine (F)
            'G': { "offset": -8, "height": 8 }, // Glycine (G)
            'H': { "offset": -8, "height": 8 }, // Histidine (H)
            'I': { "offset": -8, "height": 8 }, // Isoleucine (I)
            'K': { "offset": -8, "height": 8 }, // Lysine (K)
            'L': { "offset": -8, "height": 8 }, // Leucine (L)
            'M': { "offset": -8, "height": 8 }, // Methionine (M)
            'N': { "offset": -8, "height": 8 }, // Asparagine (N)
            'P': { "offset": -8, "height": 8 }, // Proline (P)
            'Q': { "offset": -8, "height": 8 }, // Glutamine (Q)
            'R': { "offset": -8, "height": 8 }, // Arginine (R)
            'S': { "offset": -8, "height": 8 }, // Serine (S)
            'T': { "offset": -8, "height": 8 }, // Threonine (T)
            'V': { "offset": -8, "height": 8 }, // Valine (V)
            'W': { "offset": -8, "height": 8 }, // Tryptophan (W)
            'Y': { "offset": -8, "height": 8 }, // Tyrosine (Y)
            // ASCII values, for Int8Array version
            45: { "offset": -5, "height": 2 }, // '-'
            61: { "offset": -8, "height": 8 }, // '='
            97: { "offset": -8, "height": 8 }, // Alanine (A)
            99: { "offset": -8, "height": 8 }, // Cysteine (C)
            100: { "offset": -8, "height": 8 }, // Aspartic acid (D)
            101: { "offset": -8, "height": 8 }, // Glutamic acid (E)
            102: { "offset": -8, "height": 8 }, // Phenylalanine (F)
            103: { "offset": -8, "height": 8 }, // Glycine (G)
            104: { "offset": -8, "height": 8 }, // Histidine (H)
            105: { "offset": -8, "height": 8 }, // Isoleucine (I)
            107: { "offset": -8, "height": 8 }, // Lysine (K)
            108: { "offset": -8, "height": 8 }, // Leucine (L)
            109: { "offset": -8, "height": 8 }, // Methionine (M)
            110: { "offset": -8, "height": 8 }, // Asparagine (N)
            112: { "offset": -8, "height": 8 }, // Proline (P)
            113: { "offset": -8, "height": 8 }, // Glutamine (Q)
            114: { "offset": -8, "height": 8 }, // Arginine (R)
            115: { "offset": -8, "height": 8 }, // Serine (S)
            116: { "offset": -8, "height": 8 }, // Threonine (T)
            118: { "offset": -8, "height": 8 }, // Valine (V)
            119: { "offset": -8, "height": 8 }, // Tryptophan (W)
            121: { "offset": -8, "height": 8 }, // Tyrosine (Y)
            65: { "offset": -8, "height": 8 }, // Alanine (A)
            67: { "offset": -8, "height": 8 }, // Cysteine (C)
            68: { "offset": -8, "height": 8 }, // Aspartic acid (D)
            69: { "offset": -8, "height": 8 }, // Glutamic acid (E)
            70: { "offset": -8, "height": 8 }, // Phenylalanine (F)
            71: { "offset": -8, "height": 8 }, // Glycine (G)
            72: { "offset": -8, "height": 8 }, // Histidine (H)
            73: { "offset": -8, "height": 8 }, // Isoleucine (I)
            75: { "offset": -8, "height": 8 }, // Lysine (K)
            76: { "offset": -8, "height": 8 }, // Leucine (L)
            77: { "offset": -8, "height": 8 }, // Methionine (M)
            78: { "offset": -8, "height": 8 }, // Asparagine (N)
            80: { "offset": -8, "height": 8 }, // Proline (P)
            81: { "offset": -8, "height": 8 }, // Glutamine (Q)
            82: { "offset": -8, "height": 8 }, // Arginine (R)
            83: { "offset": -8, "height": 8 }, // Serine (S)
            84: { "offset": -8, "height": 8 }, // Threonine (T)
            86: { "offset": -8, "height": 8 }, // Valine (V)
            87: { "offset": -8, "height": 8 }, // Tryptophan (W)
            89: { "offset": -8, "height": 8 } // Tyrosine (Y)
        }
      }
    },
    colour: {
        "text": "#eeeeee",
        "highlight": "#FF4444",
        "background": '#222222'
    },
    displayText: {
        // Amino acids
        97: "A",  // Alanine (A)
        99: "C",  // Cysteine (C)
        100: "D",  // Aspartic Acid (D)
        101: "E",  // Glutamic Acid (E)
        102: "F",  // Phenylalanine (F)
        103: "G",  // Glycine (G)
        104: "H", // Histidine (H)
        105: "I",  // Isoleucine (I)
        107: "K",  // Lysine (K)
        108: "L",  // Leucine (L)
        109: "M",  // Methionine (M)
        110: "N",  // Asparagine (N)
        112: "P",  // Proline (P)
        113: "Q",  // Glutamine (Q)
        114: "R",  // Arginine (R)
        115: "S", // Serine (S)
        116: "T",  // Threonine (T)
        118: "V", // Valine (V)
        119: "W",  // Tryptophan (W)
        121: "Y" , // Tyrosine (Y)
        45: "", // '-' becomes empty string
        61: "", // '=' becomes empty string
    }
  }
} ;
