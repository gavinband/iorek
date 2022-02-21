'use strict' ;
function run_msa_viewer( data ) {
	// Turn sequence strings into arrays.
	for( let i = 0; i < data.alignment.length; ++i ) {
		data.alignment[i].sequence = data.alignment[i].sequence.split( "" ) ;
	}

	let reference_name = data.alignment[ 0 ].name ;
	let msa = new MSA( data.alignment, data.ranges ) ;
	let reference = msa.scales.ungappedSequences[reference_name] ;

	let viewer = new MSAView(
		d3.select( ".figure" ),
		msa,
		reference,
		new GeneView( data.genes, reference.coordinateRange ),
		data.annotations
	) ;
	let controller = new MSAController( viewer.panels.sequences, viewer ) ;

	let redraw = function() {
		viewer.draw() ;
		window.requestAnimationFrame( redraw ) ;
	} ;
	window.requestAnimationFrame( redraw ) ;
}
