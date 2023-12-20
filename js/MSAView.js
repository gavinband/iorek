'use strict' ;

let addRange = function( range1, range2 ) {
	return [ range1[0] + range2[0], range1[1] + range2[1] ] ;
}

let getPositiveRange = function( range ) {
	return [
		Math.min( range[0], range[1] ),
		Math.max( range[0], range[1] )
	] ;
}

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
				annotation: 12,
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
	this.elt.selectAll( '*' ).remove() ;
	this.panels = {
		controls: elt.append( 'svg' ).attr( 'width', 900 ).attr( 'height', 200 ).attr( 'class', 'controls' ),
		names: elt.append( 'svg' ).attr( 'width', 900 ).attr( 'height', 600 ).attr( 'class', 'names' ),
		sequences: elt.append( 'canvas' ).attr( 'width', 900 ).attr( 'height', 600 ).attr( 'class', 'sequences' ),
		genes: elt.append( 'svg' ).attr( 'width', 900 ).attr( 'height', 200 ).attr( 'class', 'genes' )
	} ;
	this.msa = msa ;
	this.reference = reference ;
	this.reference.orientation = Math.sign( reference.coordinateRange.end - reference.coordinateRange.start ) ;
	this.reference.baseMargin = (this.reference.orientation == 1) ? [-0.5, 0.5] : [1.5, 0.5] ;
	//console.log( "R", this.reference ) ;
	this.genes = genes ;
	this.annotations = annotations ;
	this.target = "sequence" ;
	
	if( genes.count > 0 ) {
		geom.layout.heights.genes = Math.max(
			geom.layout.heights.genes,
			20 * genes.numberOfLevels
		) ;
	} else {
		geom.layout.heights.genes = 20 ;
	}

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
		// adjust ranges by half a basewidth, so that bases line up on coordinates.
			
		// bases are drawn centered on the integers.
		// leave enough space for half a base either side
		result.msaToX = new d3.scaleLinear()
			.domain( [ -0.5, msa.scales.alignmentLength - 0.5 ] )
			.range( [ geom.margin.left, geom.layout.width.sequences - geom.margin.right ] ) ;
		//console.log( "RCR", reference ) ;
		let orientation = reference.orientation ;
		// Ditto for the concatenated sequence
		result.concatenatedToX = new d3.scaleLinear()
			.domain( [ -0.5, reference.sequence.length - 0.5 ] )
			.range( result.msaToX.range() ) ;
		result.physicalToConcatenated = new d3.scaleLinear()
			.domain( [ reference.coordinateRange.start, reference.coordinateRange.end ] )
			.range( [ 0, reference.sequence.length - 1 ] ) ;
		result.physicalToX = d3.scaleLinear()
			.domain( result.physicalToConcatenated.domain() )
			.range( result.physicalToConcatenated.range().map( result.concatenatedToX )) ;
		result.genes = new d3.scaleLinear()
			.domain( [Math.max( genes.numberOfLevels, 1 ), 0] )
			.range( [geom.margin.top, geom.layout.heights.genes - geom.margin.bottom - 10 ] ) ;
		console.log( "RCR", result.concatenatedToX.domain(), result.concatenatedToX.range(), result.physicalToX.domain(), result.physicalToX.range(), result.concatenatedToX( 0 ), result.concatenatedToX( 39 ), result.physicalToX( 10 ), result.physicalToX( 49 ) ) ;
		return result ;
	}() ;

	this.aes = {
		bases: {
			colour: {
				'-': 'lightgrey',
				'm': '#333618',
				'a': '#02AAE9',
				't': '#1A356C',
				'c': '#941504',
				'g': '#F44B1A',
				'n': '#555555',
				'A': '#02AAE9',
				'T': '#1A356C',
				'C': '#941504',
				'G': '#F44B1A',
				'N': '#555555'
			},
			geom: {
				'-': { "offset": -5, "height": 2 },
				'm': { "offset": -8, "height": 8 },
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
			}
		},
		colour: {
			"text": "#eeeeee",
			"highlight": "#FF4444",
			"background": '#222222'
		}
	} ;
	
	this.drawn_viewport = [ 0, 0 ] ;
	
	console.log( "++ MSAView(): scales.reference = ", this.scales.concatenatedToX.domain(), this.scales.concatenatedToX.range() ) ;
	
	return this ;
}


MSAView.prototype.viewport = function() {
	return this.scales.msaToX.domain() ;
}

MSAView.prototype.setViewport = function( viewport ) {
	//assert( end >= start && start >= 0 ) ;
	
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
	let orientation = this.reference.orientation ;

	let refViewport = [
		lr.invert( Math.max( viewport[0], lr.range()[0] )), // clip to left edge
		rr.invert( Math.min( viewport[1], rr.range()[1] ))  // clip to right edge
	] ;

	console.log( "setViewport()", viewport ) ;
	this.scales.msaToX.domain( viewport ) ;
	let physicalRange = addRange( refViewport, this.reference.baseMargin ) ;
	this.set_physical_domain( refViewport ) ;
} ;

MSAView.prototype.set_physical_domain = function( domain ) {
	let s = this.scales ;
	let concatenated_domain = [
		Math.max( Math.floor( s.physicalToConcatenated( domain[0] )), 0 ),
		Math.min( Math.ceil( s.physicalToConcatenated( domain[1] )), this.reference.sequence.length )
	] ;
	s.concatenatedToX.domain( addRange( concatenated_domain, [-0.5, -0.5 ] )) ;
	let concatenated_domain_closed = addRange( concatenated_domain, [0,-1] ) ;
	let physical_domain = concatenated_domain_closed.map( s.physicalToConcatenated.invert ) ;
	s.physicalToX.domain( physical_domain ).range( concatenated_domain_closed.map( s.concatenatedToX ) ) ;

	console.log( "SPD", s.concatenatedToX.domain(), s.physicalToX.domain(), s.concatenatedToX.range(), s.physicalToX.range() ) ;
}

MSAView.prototype.updateLayout = function() {
	let geom = this.geom ;
	geom.layout.width.all = window.innerWidth - 40 ;
	geom.layout.width.sequences = window.innerWidth - geom.layout.width.names - 40 ;
	geom.layout.width.reference = window.innerWidth - geom.layout.width.names - 40 ;
	let visible_range = [ geom.margin.left, geom.layout.width.sequences - geom.margin.right ] ;
	this.scales.msaToX.range( visible_range ) ;
	this.scales.concatenatedToX.range( visible_range ) ;
	this.scales.physicalToX.range(
		this.scales.physicalToConcatenated.range().map( this.scales.concatenatedToX )
	) ;
}

MSAView.prototype.draw = function( force ) {
	let viewport = this.scales.msaToX.domain() ;
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
	panels.controls.attr( 'height', '30px' ) ;
	panels.controls.attr( 'width', geom.layout.width.reference ) ;

	panels.sequences.selectAll('*').remove() ;
	
	{
		let formatName = function( name ) {
			if( name == "reference" ) {
				return "(concatenated)" ;
			} else {
				return name ;
			}
		}
		let renderName = function( selection ) {
			selection
				.append( 'text' )
				.attr( 'x', 0 )
				.attr( 'y', 0 )
				.attr( 'text-anchor', 'end' )
				.attr( 'dominant-baseline', 'middle' )
				.attr( 'font-size', d => (d.track_name == 'sequence') ? '12px' : '8px' )
				.attr( 'dy', d => (d.track_name == 'sequence') ? (-aes.bases.geom['a'].height/2) : (-geom.layout.heights.annotation/2) )
				.attr( 'font-family', 'Courier' )
				.attr( 'font-style', 'italic' )
				.attr( 'fill', aes.colour.text )
				.text( d => (d.track_name == 'sequence') ? formatName( d.sequence_id ) : d.track_name ) ;
		} ;
		let names = panels.names.selectAll( 'g.name' )
			.data( vs.tracks.tracks() )
			.enter()
			.append( 'g' )
			.attr( "class", "name" )
			.attr( 'transform', ( d, i ) => ('translate(' + (geom.layout.width.names - geom.margin.right) + ',' + vs.y(vs.tracks.map(d.sequence_id, d.track_name).baseline) + ")" ))
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
			.attr( 'fill', d => this.aes.bases.colour[d] ) ;
		panels.controls.selectAll( 'g.base' )
			.append( 'text' )
			.attr( 'x', 10 )
			.attr( 'y', 0 )
			.attr( 'font-size', '10pt' )
			.attr( 'font-family', 'Palatino' )
			.attr( 'dominant-baseline', 'middle' )
			.text( d => d.toUpperCase() ) ;
			
		let switches = panels.controls.selectAll( 'g.control' )
			.data( ['highlight mismatches?'] )
			.enter()
			.append( 'g' )
			.attr( 'class', 'control' )
			.attr( 'transform', "translate(180,5)") ;
		switches
			.append( 'rect' )
			.attr( 'class', 'checkbox' )
			.attr( 'checked', 'false' )
			.attr( 'x', 0 )
			.attr( 'y', 0 )
			.attr( 'width', 8 )
			.attr( 'height', 8 )
			.attr( 'stroke', aes.colour.text )
			.attr( 'fill', '#111111' ) ;
		switches.append( 'path' )
			.attr( 'd', 'M1 1 L7 7 M7 1 L1 7')
			.attr( 'stroke', '#111111' ) ;
		switches
			.append( 'text' )
			.attr( 'x', 12 )
			.attr( 'y', 4 )
			.attr( 'alignment-baseline', 'central' )
			.attr( 'font-size', '8pt' )
			.text( d => d ) ;
	}
	{
		let genes = this.genes ;
		//console.log( "GENES", this.genes ) ;
		genes.draw(
			panels.genes, {
				position: vs.physicalToX,
				level: vs.genes
			}
		) ;
	}
	{
		let axis = panels.genes.selectAll( 'g.axis' ).data( ['axis'] ) ;
		axis
			.enter()
			.append( 'g' )
			.attr( 'class', 'axis' )
		let axisBottom = d3.axisBottom( vs.physicalToX ) ;
		panels.genes
			.selectAll( "line.verts" )
			.data( vs.physicalToX.ticks() )
			.join( "line" )
			.attr( "class", "verts" )
			.attr( "x1", vs.physicalToX )
			.attr( "x2", vs.physicalToX )
			.attr( "y1", 0 )
			.attr( "y2", vs.genes.range()[1] )
			.attr( "stroke", "rgba( 255, 255, 255, 0.2 )" )
		;
		panels.genes.selectAll( 'g.axis' )
			.attr( 'transform', 'translate( 0,' + ( vs.genes.range()[1] + 10 ) + ')' )
			.call( axisBottom ) ;
	}

	let drawSequence = function( sequence, levels, y, baseWidth, xScale, range, ctx ) {
		// calculate stride across sequence as follows.
		// Fir, we aim to have every drawn rectangle of some minimum size below which
		// we aggregate.  
		let visibleMarkWidth = 3.0 ;
		// Our aggregation happens at power-of-2 sizes (c.f. MSA.js / computeLevels)
		// Therefore we compute the power-of-two stride that is less than or equal to the
		// number of bases that fit into the visible width.
		let by = Math.pow(
			2,
			Math.floor(
				Math.max(
					0,
					Math.log2( visibleMarkWidth / baseWidth )
				)
			)
		) ;
		by = Math.min( by, 1024 ) ;
		//by = Math.max( 1, Math.floor( 1.0/baseWidth )) ;
		// leave a small gap between bases, but
		// never draw less than one pixel wide
		let visualBaseWidth = Math.max( baseWidth * ( by - 0.02 ), 1 ) ;
		let j = range[0] ;
		let level_j = Math.floor( j/by ) ;
		let base = 'A' ;
		for( ; j < range[1]; j += by, level_j += 1 ) {
			if( by > 1 && levels !== null ) {
				base = levels[by][level_j] ;
			} else {
				base = sequence[j] ;
			}
			let geom ;
			if( aes.bases.geom.hasOwnProperty( base )) {
				geom = aes.bases.geom[base] ;
			} else {
				geom = aes.bases.geom['-'] ;
			}
			ctx.fillStyle = aes.bases.colour[base] || aes.bases.colour['-'];
			ctx.fillRect(
				xScale( j ) - visualBaseWidth / 2,
				y + geom.offset,
				visualBaseWidth,
				geom.height
			) ;
		}
	} ;

	{
		// MSA
		let sequences = panels.sequences.node() ;
		let ctx = sequences.getContext( '2d' ) ;
		let baseWidth = (vs.msaToX(1) - vs.msaToX(0)) ;
		let msa = this.msa ;
		// only draw bases in the viewport
		let visible_range = [
			Math.max( Math.floor( viewport[0] ), 0 ),
			Math.min( Math.ceil( viewport[1] + 1 ), msa.alignment[0].sequence.length )
		] ;
		for( let i = 0; i < msa.alignment.length; ++i ) {
			let sequence = msa.alignment[i].sequence ;
			drawSequence(
				msa.alignment[i][ this.target ],
				msa.alignment[i][ this.target ].levels,
				vs.y(vs.tracks.map(msa.alignment[i].name, "sequence").baseline),
				baseWidth,
				j => vs.msaToX( msa.scales.global(j)),
				visible_range,
				ctx
			) ;
		}
		
		// Concatenated sequence
		let orientation = this.reference.orientation ;
		let baseOffset = ((orientation==-1)?-1:0) ;
		//console.log( "DRAWING REFERENCE, starting at ", vs.concatenatedToX(this.reference.coordinateRange.start )) ;
		let referenceBaseWidth = Math.abs(
			vs.concatenatedToX(this.reference.coordinateRange.start+orientation)
			- vs.concatenatedToX(this.reference.coordinateRange.start)
		) ;
		drawSequence(
			this.reference.sequence,
			null,
			vs.y(vs.tracks.map( "reference", "sequence" ).baseline),
			referenceBaseWidth,
			this.scales.concatenatedToX,
			[ 0, this.reference.sequence.length ],
			ctx
		) ;
		
		// Lines joining concatenated sequence and MSA
		{
			let gs = msa.scales.global ;
			let yoffset = 2 ;
			let ys = {
				"a": vs.y(vs.tracks.map( this.reference.name, "sequence" ).baseline) + yoffset,
				"b": vs.y(vs.tracks.map( "reference", "sequence" ).baseline) - geom.layout.heights.base - yoffset
 			} ;
			// plot indels between joined and msa reference
			// and plot axis tick points

			let ticks = vs.physicalToX.ticks() ;
			let last_x00 = -100000000 ;
			for( let i = 0; i < this.reference.ranges.length; ++i ) {
				let range = this.reference.ranges[i] ;
				console.log( "R", range ) ;
				let x00 = vs.msaToX( gs( range.inAlignment.start )) - baseWidth / 2 ;
				if( (x00 - last_x00) > 5 ) {
					let x10 = vs.msaToX( gs( range.inAlignment.end )) - baseWidth / 2;
					let x01 = vs.physicalToX( range.inSequence.start ) - referenceBaseWidth / 2 ;
					let x11 = vs.physicalToX( range.inSequence.end )  - referenceBaseWidth / 2 ;

					ctx.beginPath() ;
					// ref sequence is 2 * heights.sequence down.
					ctx.moveTo( x00, ys.a ) ;
					ctx.lineTo( x01, ys.b ) ;
					ctx.moveTo( x11, ys.b ) ;
					ctx.lineTo( x10, ys.a ) ;
					ctx.strokeStyle = 'rgba(255, 255, 255, 0.7 )' ;
					ctx.stroke() ;
					last_x00 = x00 ;

					let pr = getPositiveRange( [ range.inSequence.start, range.inSequence.end ] ) ;
					// only take ticks in range, and not too close to the ends to avoid
					// multiple similar-looking lines.
					let rangeTicks = ticks.filter( t => (t >= (pr[0] + 2) && t < (pr[1]-2)) ) ;
					console.log( range.inAlignment, range.inSequence, rangeTicks ) ;
					ctx.save() ;
					ctx.strokeStyle = 'rgba(255, 255, 255, 0.2 )' ;
					//ctx.strokeStyle = "#3b3f46" ;
					ctx.lineWidth = 1 ;
					//ctx.setLineDash( [2,2] ) ;
					for( let j = 0; j < rangeTicks.length; ++j ) {
						let x0 = vs.msaToX( range.inAlignment.start + Math.abs(rangeTicks[j] - range.inSequence.start) ) ;
						let x1 = vs.physicalToX( rangeTicks[j] ) ;
						ctx.beginPath() ;
						// ref sequence is 2 * heights.sequence down.
						ctx.moveTo( x0, ys.a ) ;
						ctx.lineTo( x1, ys.b ) ;
						ctx.stroke() ;
					}
					ctx.restore() ;
				}
			}
		}
	}
	
	{
		let baseWidth = Math.abs( (vs.msaToX(1) - vs.msaToX(0))) ;
		let orientation = this.reference.orientation ;
		let sequences = panels.sequences.node() ;
		let ctx = sequences.getContext( '2d' ) ;
		for( let sequence_id in this.annotations ) {
			let sequence_annotations = this.annotations[sequence_id] ;
			let ranges = this.msa.scales.ranges[sequence_id] ;
			for( let track_name in sequence_annotations ) {
				let track = sequence_annotations[track_name] ;
				ctx.beginPath() ;
				ctx.moveTo(
					geom.margin.left,
					vs.y( vs.tracks.map( sequence_id, track_name ).baseline )
				) ;
				ctx.lineTo(
					geom.layout.width.all - geom.margin.right,
					vs.y( vs.tracks.map( sequence_id, track_name ).baseline )
				) ;
				ctx.lineWidth = 0.5 ;
				ctx.strokeStyle = '#558855' ;
				ctx.stroke() ;
				for( let i = 0; i < ranges.length; ++i ) {
					let sequenceToAlignment = ranges[i] ;
					let domain = getPositiveRange( sequenceToAlignment.domain() );
					let range_track = track.filter( elt => elt.end >= domain[0] && elt.start <= domain[1] ) ;
					for( let j = 0; j < range_track.length; ++j ) {
						let a = range_track[j] ;
						let rd = {
							x1: vs.msaToX( sequenceToAlignment( Math.max( a.start, domain[0] ))) - orientation * (baseWidth/2),
							y1: vs.y( vs.tracks.map( sequence_id, track_name ).baseline + vs.tracks.scale_value( sequence_id, track_name, a.value ) ),
							x2: vs.msaToX( sequenceToAlignment( Math.min( a.end, domain[1] ))) + orientation * (baseWidth/2),
							y2: vs.y( vs.tracks.map( sequence_id, track_name ).baseline)
						} ;
						rd.xl = Math.min( rd.x1, rd.x2 ) ;
						rd.xr = Math.max( rd.x1, rd.x2 ) ;
						//console.log( "RD", rd ) ;
						ctx.fillStyle = '#6a6a6a' ;
						ctx.fillRect(
							rd.xl,
							rd.y1,
							rd.xr - rd.xl,
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
