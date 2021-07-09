library( RSQLite )

N = 1000
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

system( "sqlite3 -header -csv model.sqlite 'SELECT * FROM Training' > training_all.csv" )
system( "sqlite3 -header -csv model.sqlite 'SELECT * FROM Coverage' > coverage_all.csv" )

