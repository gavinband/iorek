'use strict' ;

// Object that acts like a bit like D3 scaleBand but gives us back bands for a
// sequence and multiple annotation tracks, depending on what annotations are
// provided. Any padding is to be added inside these track heights.
let AlignmentTrackScale = function(
	msa,
	annotations,
	geom
) {
	this.msa = msa ;
	this.annotations = annotations ;
	let sequence_ids = [ "reference", "" ].concat( msa.sequence_ids()) ;
	let totalHeight = geom.layout.height - geom.margin.top ;
	this.sizes = function( sequence_ids, annotations ) {
		let result = {} ;
		let global_baseline = 0 ;
		for( let i = 0; i < sequence_ids.length; ++i ) {
			let sequence_id = sequence_ids[i] ;
			let baseline = 0 ;
			result[sequence_id] = {
				baseline: global_baseline,
				height: geom.layout.heights.sequence,
				tracks: {
					"sequence": {
						"baseline": global_baseline + baseline,
						"relative_baseline": baseline,
						"height": geom.layout.heights.sequence
					}
				}
			} ;
			baseline += result[sequence_id].height ;
			if( sequence_id in annotations ) {
				let id_annotations = annotations[sequence_id] ;
				for( let track_name in id_annotations ) {
					let track = id_annotations[track_name] ;
					let minmax = [1000000,-100000] ;
					for( let z = 0; z < track.length; ++z ) {
						minmax[0] = Math.min( minmax[0], track[z].value ) ;
						minmax[1] = Math.max( minmax[1], track[z].value ) ;
					}
					minmax[0] = 0 ;
					result[sequence_id].tracks[track_name] = {
						"baseline": baseline + global_baseline,
						"relative_baseline": baseline,
						"height": geom.layout.heights.annotation,
						"value_range": minmax
					} ;
					result[sequence_id].height += geom.layout.heights.annotation ;
					baseline += geom.layout.heights.annotation ;
				}
			}
			result[sequence_id].height = Math.max( result[sequence_id].height, geom.layout.heights.min_per_sequence ) ;
			global_baseline += result[sequence_id].height ;
		}
		return result ;
	}( sequence_ids, annotations ) ;

	console.log( "AlignmentTrackScale(): ", this.sizes ) ;
	return this ;
}

AlignmentTrackScale.prototype.total_height = function() {
	let result = 0 ;
	for( name in this.sizes ) {
		result += this.sizes[name].height ;
	} ;
	return result ;
}

// return the absolute and relative baseline for the track, and its height.
AlignmentTrackScale.prototype.map = function( sequence_id, track_name ) {
	return this.sizes[ sequence_id ].tracks[ track_name ] ;
}

// return the absolute and relative baseline for the track, and its height.
AlignmentTrackScale.prototype.tracks = function() {
	let result = [] ;
	for( let sequence_id in this.sizes ) {
		for( let track_name in this.sizes[sequence_id].tracks ) {
			result.push({sequence_id: sequence_id, track_name: track_name }) ;
		}
	}
	return result ;
}

AlignmentTrackScale.prototype.scale_value = function( sequence_id, track_name, value, margin = 2 ) {
	let spec = this.sizes[ sequence_id ].tracks[ track_name ] ;
	return ( spec.height - margin ) * ( value - spec.value_range[0]) / (spec.value_range[1] - spec.value_range[0] ) ;
}
