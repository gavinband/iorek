library( RSQLite )

N = 100
S = 100

model = list(
	normal =		list( frequency = 0.65, profile = rep( 1, S )),
	DEL1 = 		list( frequency = 0.20, profile = c( rep( 1, 20 ), rep( 0, 40 ), rep( 1, 40 ))),
	DEL2 = 		list( frequency = 0.10, profile = c( rep( 1, 40 ), rep( 0, 40 ), rep( 1, 20 ))),
	DUP1 = 		list( frequency = 0.05, profile = c( rep( 1, 30 ), rep( 2, 15 ), rep( 1, 55 )))
)

#
frequencies = sapply( model, function(s) { s$frequency } )
cumulative = cumsum( c(0,frequencies ))
means = rnorm( N, mean = 20, sd = 3 )
variances = means/3

haplotypes = matrix(
	sapply( runif( 2 * N ), function(x) { max( which( cumulative < x )) } ),
	ncol = 2,
	dimnames = list( sprintf( "sample_%d", 1:N ), sprintf( "hap_%d", 0:1 ))
)

profiles = matrix(
	NA,
	nrow = S,
	ncol = N,
	dimnames = list( sprintf( "site_%d", 1:S ), sprintf( "sample_%d", 1:N ))
)
coverage = profiles
training = profiles
ploidy = 2
for( i in 1:N ) {
	training[,i] = pmax( rnorm( n = S, mean = means[i] * ploidy, sd = sqrt(variances[i] * ploidy) + 0.05 ), 0 ) * 300
	profiles[,i] = model[[haplotypes[i,1]]]$profile + model[[haplotypes[i,2]]]$profile
	coverage[,i] = pmax( rnorm( n = S, mean = means[i] * profiles[,i], sd = sqrt(variances[i] * profiles[,i]) + 0.05 ), 0 ) * 300
}
colnames(training) = sprintf( "%s:coverage", colnames(training))
colnames(coverage) = sprintf( "%s:coverage", colnames(coverage))
mode(haplotypes) = "integer"
db = dbConnect( dbDriver( "SQLite" ), "model.sqlite" )
dbWriteTable(
	db,
	"Haplotypes",
	data.frame( sample = rownames( haplotypes ), haplotypes-1 ),
	row.names = FALSE,
	overwrite = T
)

dbWriteTable(
	db,
	"Coverage",
	cbind(
		data.frame(
			chromosome = '1',
			position = as.integer( seq( from = 1000, length = S, by = 300 )),
			size = as.integer(300),
			N = as.integer(300)
		),
		as.data.frame(coverage)
	),
	row.names = F,
	overwrite = T
)

dbWriteTable(
	db,
	"Training",
	cbind(
		data.frame(
			chromosome = '1',
			position = as.integer( seq( from = 1000, length = S, by = 300 )),
			size = as.integer(300),
			N = as.integer(300)
		),
		as.data.frame(training)
	),
	row.names = F,
	overwrite = T
)

system(
	"sqlite3 -header -csv model.sqlite 'SELECT chromosome, position, size, N, `sample_1:coverage`, `sample_2:coverage`, `sample_3:coverage`, `sample_4:coverage`, `sample_5:coverage`, `sample_6:coverage`, `sample_7:coverage`, `sample_8:coverage`, `sample_9:coverage`, `sample_10:coverage` FROM Training ;' > training_1_to_10.csv"
)
system(
	"sqlite3 -header -csv model.sqlite 'SELECT chromosome, position, size, N, `sample_1:coverage`, `sample_2:coverage`, `sample_3:coverage`, `sample_4:coverage`, `sample_5:coverage`, `sample_6:coverage`, `sample_7:coverage`, `sample_8:coverage`, `sample_9:coverage`, `sample_10:coverage` FROM Coverage ;' > coverage_1_to_10.csv"
)

system(
	"sqlite3 -header -csv model.sqlite 'SELECT chromosome, position, size, N, `sample_1:coverage`, `sample_2:coverage`, `sample_3:coverage`, `sample_4:coverage`, `sample_5:coverage`, `sample_6:coverage`, `sample_7:coverage`, `sample_8:coverage`, `sample_9:coverage`, `sample_10:coverage`, `sample_11:coverage`, `sample_12:coverage`, `sample_13:coverage`, `sample_14:coverage`, `sample_15:coverage`, `sample_16:coverage`, `sample_17:coverage`, `sample_18:coverage`, `sample_19:coverage`, `sample_20:coverage` FROM Training ;' > training_1_to_20.csv"
)
system(
	"sqlite3 -header -csv model.sqlite 'SELECT chromosome, position, size, N, `sample_1:coverage`, `sample_2:coverage`, `sample_3:coverage`, `sample_4:coverage`, `sample_5:coverage`, `sample_6:coverage`, `sample_7:coverage`, `sample_8:coverage`, `sample_9:coverage`, `sample_10:coverage`, `sample_11:coverage`, `sample_12:coverage`, `sample_13:coverage`, `sample_14:coverage`, `sample_15:coverage`, `sample_16:coverage`, `sample_17:coverage`, `sample_18:coverage`, `sample_19:coverage`, `sample_20:coverage` FROM Coverage ;' > coverage_1_to_20.csv"
)

system( "sqlite3 -header -csv model.sqlite 'SELECT * FROM Training' > training_all.csv" )
system( "sqlite3 -header -csv model.sqlite 'SELECT * FROM Coverage' > coverage_all.csv" )
system( "sqlite3 -header -csv model.sqlite 'SELECT sample, (MIN(hap_0,hap_1) || \'/\' || MAX(hap_0,hap_1)) AS genotype FROM Haplotypes' > haplotypes.csv" )


# PLot it
pdf( file = "first_20.pdf", width = 8, height = 8 )
layout( matrix( 1:21, ncol = 1 ), heights = c( rep( 1, 20 ), 0.75 ))
par( mar = c( 0.1, 1, 0.1, 1 ))
for( i in 1:20 ) {
	m = means[i]
	plot( 1:S, coverage[,i] / 300,
		xlab = '', ylab = "coverage",
		xaxt = 'n', yaxt = 'n',
		main = '',
		xlim = c( 1, S ),
		ylim = c( 0, m * 4 ),
		col = 'black',
		pch = 19,
		cex = 0.75,
		type = 'l'
	)
	abline( h = m * 0:4, col = c( 'grey', 'grey', 'red', 'grey', 'grey' ), lty = 2 )
	axis(2, at = m * 0:4, label = 0:4 )
	legend(
		"topleft",
		legend = i,
		bty = 'n'
		)
}
par( mar = c( 2, 1, 0.1, 1 ))
plot(0, 0, col = 'white', xlab = '', ylab = '', bty = 'n', xaxt = 'n', yaxt = 'n', xlim = c( 1, S ) )
axis(1)
dev.off()




