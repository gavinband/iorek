'use strict' ;

let MSAController = function( panels, view ) {
	this.panels = panels ;
	this.view = view ;
	this.drag = { start: 0 } ;
	let self = this ;
	let doDrag = function( x ) {
		let here = self.view.scales.alignmentToX.invert( x ) ;
		let drag = here - self.drag.start ;
		let viewport = self.view.viewport() ;
		drag = Math.min( drag, viewport[0] - self.view.msa.scales.global.range()[0] + 50 ) ;
		drag = Math.max( drag, viewport[1] - self.view.msa.scales.global.range()[1] - 50 ) ;
		//console.log( "     DRAG", self.drag, self.viewport, self.view.msa.scales.global.range(), drag ) ;
		self.view.setViewport( [ viewport[0] - drag, viewport[1] - drag ] ) ;
	} ;
	
	panels.controls.on( "click", function( e ) {
		let checkbox = d3.select( e.target ) ;
		if( checkbox.attr( 'checked' ) == 'false' ) {
			checkbox.attr( 'checked', 'true' ) ;
			checkbox.attr( 'fill', '#dddddd' ) ;
			self.view.target = "mismatches" ;
		} else {
			checkbox.attr( 'fill', '#111111' ) ;
			checkbox.attr( 'checked', 'false' ) ;
			self.view.target = "sequence" ;
		}
		self.view.draw( true ) ;
	}) ;
	
	// set up dragging
	panels.sequences.on( "mousedown", function( e ) {
//			self.dragging = true ;
//			self.dragStartRegion = self.viewport ;
//			self.dragStartCentre = (self.viewport[1] + self.viewport[0])/2 ;
		self.drag.start = self.view.scales.alignmentToX.invert( e.offsetX ) ;
		self.dragging = true ;
		//console.log( "START DRAG", e.offsetX, e.offsetY, self.drag ) ;
	}) ;
	panels.sequences.on( "mousemove", function( e ) {
		if( self.dragging ) {
			doDrag( e.offsetX ) ;
		}
	}) ;
	panels.sequences.on( "mouseup", function(e) {
		if( self.dragging ) {
			doDrag( e.offsetX ) ;
			self.dragging = false ;
		}
	}) ;
	panels.sequences.on( "mouseout", function(e) {
		if( self.dragging ) {
			doDrag( e.offsetX ) ;
			self.dragging = false ;
		}
	}) ;
	
	let zoomRegion = function( viewport, focus, zoom ) {
		// Expand or contract region around given centre point
		// which must lie in the region.
		// The way this works is that the chosen point stays fixed on the screen
		// And everything else zooms.
		// zoom=1 means no zoom
		// zoom>1 means zoom in
		// zoom<1 means zoom out
		return(
				[
					focus + ((viewport[0] - focus) / zoom),
					focus + ((viewport[1] - focus) / zoom)
				]
		) ;
	} ;
	// Set up zooming
	panels.sequences.on( 'mousewheel', function(e) {
		let viewport = self.view.viewport() ;
		let focus = self.view.scales.alignmentToX.invert( e.offsetX ) ;
	//	console.log( "zooming:", e.wheelDeltaY, viewport, focus, self.view.msa ) ;
		
		let scale = Math.pow( 2, Math.min( Math.abs(e.wheelDeltaY), 50.0 ) / 50.0 ) ;
		if( ( e.wheelDeltaY > 0 ) && (viewport[1] - viewport[0]) > 25 ) {
			viewport = zoomRegion( viewport, focus, scale ) 
		}
		else if( e.wheelDeltaY < 0 && (viewport[1] - viewport[0]) < ( self.view.msa.alignmentLength )) {
			viewport = zoomRegion( viewport, focus, 1/scale ) ;
		}
		viewport[0] = Math.max( viewport[0], -0.5 ) ;
		viewport[1] = Math.min( viewport[1], self.view.msa.alignmentLength + 0.5 ) ;
		self.view.setViewport( viewport ) ;
		e.stopPropagation() ;
		return false ;
	}) ;

	window.addEventListener( 'resize', function() {
		self.view.draw( true ) ;
	} ) ;

	return this ;
} ;
