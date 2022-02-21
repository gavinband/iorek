'use strict' ;

let MSAView = function(
	elt,
	msa,
	reference,
	genes,
	annotations,
	geom = {
		layout: {
			heights: {
				base: 8,
				sequence: 10,
				annotation: 10,
				genes: 100,
				min_per_sequence: 15
			},
			width: {
				all: window.innerWidth - 40,
				names: 200,
				sequences: window.innerWidth - 240,
				reference: window.innerWidth - 240
			}
		},
		margin: {
			top: 20,
			bottom: 20,
			left: 10,
			right: 10
		}
	}
) {
	this.elt = elt ;
	this.panels = {
		controls: elt.select( "svg.controls" ),
		names: elt.select( "svg.names" ),
		sequences: elt.select( "canvas.sequences" ),
		genes: elt.select( "svg.genes" )
	} ;
	this.msa = msa ;
	this.reference = reference ;
	this.genes = genes ;
	this.annotations = annotations ;

	this.geom = geom ;

	this.scales = function(){
		let result = {} ;
		result.tracks = new AlignmentTrackScale(
			msa,
			annotations,
			geom
		) ;
		result.y = new d3.scaleLinear()
			.domain( [ 0, result.tracks.total_height() ])
			.range( [ 0, result.tracks.total_height() ] ) ;
		result.x = new d3.scaleLinear()
			.domain( msa.scales.global.range() )
			.range( [ geom.margin.left, geom.layout.width.sequences - geom.margin.right ] ) ;
		result.reference = new d3.scaleLinear()
			.domain( [ reference.coordinateRange.start, reference.coordinateRange.end ] )
			.range( result.x.range() ) ;
		result.alignmentReferenceMap = new d3.scaleLinear()
			.domain( msa.scales.global.range() )
			.range( result.reference.domain() )
		;
		result.genes = new d3.scaleLinear()
			.domain( [Math.max( genes.numberOfLevels, 1 ), 0] )
			.range( [geom.margin.top, geom.layout.heights.genes - geom.margin.bottom ] ) ;
		return result ;
	}() ;

	this.aes = {
		bases: {
			'-': { 'colour': 'lightgrey', "offset": -5, "height": 2 },
			'a': { 'colour': '#02AAE9', "offset": -8, "height": 8 },
			't': { 'colour': '#1A356C', "offset": -8, "height": 8 },
			'c': { 'colour': '#F44B1A', "offset": -8, "height": 8 },
			'g': { 'colour': '#941504', "offset": -8, "height": 8 }
		},
		colour: {
			"text": "#eeeeee",
			"highlight": "#FF4444",
			"background": '#222222'
		}
	} ;
	
	this.drawn_viewport = [ 0, 0 ] ;
	
	return this ;
}

MSAView.prototype.setViewport = function( viewport ) {
	//assert( end >= start && start >= 0 ) ;
	this.scales.x.domain( viewport ) ;
	let ranges = this.msa.scales.ranges[ this.reference.name ] ;
	let choice = { left: -1, right: -1 } ;
	for( let i = 0; i < ranges.length; ++i ) {
		let range = ranges[i] ;
		if( range.range()[1] >= viewport[0] && choice.left == -1 ) {
			choice.left = i ;
		}
		if( range.range()[0] < viewport[1] ) {
			choice.right = i ;
		}
	}
	let lr = ranges[choice.left] ;
	let rr = ranges[choice.right] ;
	
	let refViewport = [
		lr.invert( Math.max( viewport[0], lr.range()[0] )),
		rr.invert( Math.min( viewport[1], rr.range()[1] ))
	] ;

	this.scales.reference.domain( refViewport ) ;
} ;

MSAView.prototype.viewport = function() {
	// current view coordinates, in msa coordinates
	return this.scales.x.domain() ;
} ;

MSAView.prototype.updateLayout = function() {
	let geom = this.geom ;
	geom.layout.width.all = window.innerWidth - 40 ;
	geom.layout.width.sequences = window.innerWidth - geom.layout.width.names - 40 ;
	geom.layout.width.reference = window.innerWidth - geom.layout.width.names - 40 ;
	this.scales.x.range( [ geom.margin.left, geom.layout.width.sequences - geom.margin.right ] ) ;
	this.scales.reference.range( this.scales.x.range() ) ;
}

MSAView.prototype.draw = function( force ) {
	let viewport = this.viewport() ;
	if( !force && (this.drawn_viewport[0] == viewport[0] && this.drawn_viewport[1] == viewport[1] )) {
		return ;
	}
	let panels = this.panels ;
	// Ensure we fit the screen.
	this.updateLayout() ;
	let geom = this.geom ;
	let vs = this.scales ;
	let aes = this.aes ;

	let canvas_height = vs.tracks.total_height() + geom.margin.top + geom.margin.bottom  ;

	vs.y.range( [ canvas_height - geom.margin.bottom, geom.margin.top ] ) ;
	// put panels in the right place
	d3.select( '.figure' )
		.attr( 'width', geom.layout.width.all )
		.attr( 'height', canvas_height + 300 ) ;

	panels.names.attr( 'width', geom.layout.width.names ) ;
	panels.names.attr( 'height', canvas_height ) ;

	panels.sequences.attr( 'width', geom.layout.width.sequences ) ;
	panels.sequences.attr( 'height', canvas_height ) ;

	panels.genes.style( 'top', canvas_height + 40 ) ;
	panels.genes.attr( 'width', geom.layout.width.reference ) ;

	panels.controls.style( 'top', '20px' ) ;
	panels.controls.attr( 'height', '20px' ) ;
	panels.controls.attr( 'width', geom.layout.width.reference ) ;

	panels.sequences.selectAll('*').remove() ;
	
	{
		let renderName = function( selection ) {
			selection
				.append( 'text' )
				.attr( 'x', 0 )
				.attr( 'y', 0 )
				.attr( 'text-anchor', 'end' )
				.attr( 'dominant-baseline', 'middle' )
				.attr( 'font-size', '12px')
				.attr( 'dy', -aes.bases['a'].height/2 )
				.attr( 'font-family', 'Courier' )
				.attr( 'font-style', 'italic' )
				.attr( 'fill', aes.colour.text )
				.text( name => name ) ;
		} ;
		let names = panels.names.selectAll( 'g.name' )
			.data( data.alignment.map( elt => elt.name ).concat( [ "reference" ]) )
			.enter()
			.append( 'g' )
			.attr( "class", "name" )
			.attr( 'transform', ( name, i ) => ('translate(' + (geom.layout.width.names - geom.margin.right) + ',' + vs.y(vs.tracks.map(name, "sequence").baseline) + ")" ))
		;
		names.call( renderName ) ;
	}

	{
		let guide = panels.controls.selectAll( 'g.guide' )
			.data( ['guide'] )
			.enter()
			.append( 'g' )
			.attr( 'class', 'guide' )
			.attr( 'transform', (d,i) => ('translate(' + (geom.layout.width.reference/2) + ", 10)" ))
			.append( 'text' ) ;
		guide
			.append( 'tspan' )
			.attr( 'font-size', '10pt' )
			.attr( 'font-family', 'Palatino' )
			.attr( 'dominant-baseline', 'middle' )
			.attr( 'text-anchor', 'middle' )
			.text( "Use mouse to scroll and zoom" )
		;
	}
	{
		let logo = panels.controls.selectAll( 'g.logo' )
			.data( ['seemsa'] )
			.enter()
			.append( 'g' )
			.attr( 'class', 'logo' )
			.attr( 'transform', (d,i) => ('translate(' + (geom.layout.width.reference - 100) + ", 10)" ))
			.append( 'text' ) ;
		logo
			.append( 'tspan' )
			.attr( 'font-size', '10pt' )
			.attr( 'font-family', 'Helvetica' )
			.attr( 'dominant-baseline', 'middle' )
			.text( d => d ) ;
		logo
			.append( 'tspan' )
			.attr( 'font-size', '6pt' )
			.attr( 'font-family', 'Palatino' )
			.attr( 'font-style', 'italic' )
			.attr( 'dominant-baseline', 'middle' )
			.attr( 'baseline-shift', 'super' )
			.text( "beta" ) ;

		let base_aes = this.aes.bases ;
		panels.controls.selectAll( 'g.base' )
			.data( [ 'a', 'c', 'g', 't' ] )
			.enter()
			.append( 'g' )
			.attr( 'class', 'base' )
			.attr( 'transform', (d,i) => ('translate(' + (20 + i*40) + ", 10)" )) ;
		panels.controls.selectAll( 'g.base' )
			.append( 'rect' )
			.attr( 'x', 0 )
			.attr( 'y', -5 )
			.attr( 'width', 6 )
			.attr( 'height', 10 )
			.attr( 'fill', d => base_aes[d].colour ) ;
		panels.controls.selectAll( 'g.base' )
			.append( 'text' )
			.attr( 'x', 10 )
			.attr( 'y', 0 )
			.attr( 'font-size', '10pt' )
			.attr( 'font-family', 'Palatino' )
			.attr( 'dominant-baseline', 'middle' )
			.text( d => d.toUpperCase() ) ;
	}
	{
		let genes = this.genes ;
		
		genes.draw(
			panels.genes, {
				position: vs.reference,
				level: vs.genes
			}
		) ;
		let axis = panels.genes.selectAll( 'g.axis' ).data( ['axis'] ) ;
		axis
			.enter()
			.append( 'g' )
			.attr( 'class', 'axis' )
		panels.genes.selectAll( 'g.axis' )
			.attr( 'transform', 'translate( 0,' + ( vs.genes.range()[1] - geom.margin.bottom + 2 ) + ')' )
			.call( d3.axisBottom( vs.reference )) ;
	}

	let drawSequence = function( sequence, y, baseWidth, xScale, lower, upper, ctx ) {
		let by = Math.max( 1, Math.floor( 1.0/baseWidth )) ;
		for( let j = lower; j < upper; j += by ) {
			let base = sequence[j] ;
			let base_aes = aes.bases[base] ;
			ctx.fillStyle = base_aes.colour ;
			ctx.fillRect(
				xScale( j ),
				y + base_aes.offset,
				Math.max( baseWidth, 1 ),
				base_aes.height
			) ;
		}
	} ;

	{
		let sequences = panels.sequences.node() ;
		let ctx = sequences.getContext( '2d' ) ;
		let baseWidth = vs.x(1) - vs.x(0) ;
		let msa = this.msa ;
		for( let i = 0; i < msa.alignment.length; ++i ) {
			let sequence = msa.alignment[i].sequence ;
			drawSequence(
				msa.alignment[i].sequence,
				vs.y(vs.tracks.map(msa.alignment[i].name, "sequence").baseline),
				baseWidth,
				j => vs.x( msa.scales.global(j)),
				Math.max( Math.floor( viewport[0] ), 0 ),
				Math.min( Math.ceil( viewport[1] ), sequence.length ),
				ctx
			) ;
		}
		
		drawSequence(
			this.reference.sequence,
			vs.y(vs.tracks.map( "reference", "sequence" ).baseline),
			vs.reference(1) - vs.reference(0),
			j => vs.reference(this.reference.coordinateRange.start + j),
			0,
			this.reference.sequence.length,
			ctx
		) ;
		
		{
			let gs = msa.scales.global ;
			let yoffset = 2 ;
			let ys = {
				"a": vs.y(vs.tracks.map( this.reference.name, "sequence" ).baseline) + yoffset,
				"b": vs.y(vs.tracks.map( "reference", "sequence" ).baseline) - geom.layout.heights.base - yoffset
 			} ;
			for( let i = 0; i < this.reference.ranges.length; ++i ) {
				let range = this.reference.ranges[i] ;
				let x00 = vs.x( gs( range.inAlignment.start )) ;
				let x01 = vs.reference( range.inSequence.start ) ;
				let x11 = vs.reference( range.inSequence.end ) ;
				let x10 = vs.x( gs( range.inAlignment.end )) ;
				ctx.beginPath() ;
				// ref sequence is 2 * heights.sequence down.
				ctx.moveTo( x00, ys.a ) ;
				ctx.lineTo( x01, ys.b ) ;
				ctx.moveTo( x11, ys.b ) ;
				ctx.lineTo( x10, ys.a ) ;
				ctx.strokeStyle = "grey" ;
				ctx.stroke() ;
			}
		}
	}
	
	{
		let sequences = panels.sequences.node() ;
		let ctx = sequences.getContext( '2d' ) ;
		for( let sequence_id in this.annotations ) {
			let sequence_annotations = this.annotations[sequence_id] ;
			let ranges = this.msa.scales.ranges[sequence_id] ;
			for( let track_name in sequence_annotations ) {
				let track = sequence_annotations[track_name] ;
				vs.tracks.map( sequence_id, track_name )
				for( let i = 0; i < ranges.length; ++i ) {
					let sequenceToAlignment = ranges[i] ;
					let domain = sequenceToAlignment.domain() ;
					let range_track = track.filter( elt => elt.end >= domain[0] && elt.start <= domain[1] ) ;
					for( let j = 0; j < range_track.length; ++j ) {
						let a = range_track[j] ;
						let rd = {
							x1: vs.x( sequenceToAlignment( Math.max( a.start, domain[0] ))),
							y1: vs.y( vs.tracks.map( sequence_id, track_name ).baseline + vs.tracks.scale_value( sequence_id, track_name, a.value ) ),
							x2: vs.x( sequenceToAlignment( Math.min( a.end, domain[1] ))),
							y2: vs.y( vs.tracks.map( sequence_id, track_name ).baseline)
						} ;
						ctx.fillStyle = '#6a6a6a' ;
						ctx.fillRect(
							rd.x1,
							rd.y1,
							rd.x2 - rd.x1,
							rd.y2 - rd.y1
						) ;
					}
				} 
			}
		}
	}
	
	this.drawn_viewport[0] = viewport[0] ;
	this.drawn_viewport[1] = viewport[1] ;
}
