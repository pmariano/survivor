use strict;
use warnings;
use AI::Pathfinding::AStar::Rectangle;
use Algorithm::Combinatorics qw(combinations);
use Storable qw(dclone);

my $w = 23;
my $h = 17;

my @entry = (
	[10,1],
	[1,14],
	[10,14],
	[20,14],
);

my @task = combinations([@entry],2);
# print join ';', map {join ',', map { "@$_"}  @$_} combinations([@entry],2);

my $max_score = -1;
my $winner = {};

my $map = AI::Pathfinding::AStar::Rectangle->new({height=>$h, width=>$w});

while(1)
{
	$map->foreach_xy_set( sub { 
		scalar( grep {$a == $_->[0] && $b == $_->[1] } @entry ) ||
		$a !=0 && $a != $w-2 && $b != 0 && $b != $h-2 &&
		int(rand(3)) != 0 
	} );

	my @path = map {
		my ($start, $end) = @$_;
		my $path = $map->astar(
			$start->[0], $start->[1],
			$end->[0], $end->[1],
		);
		$path
	} @task;

	my $score = length join "", @path;

	if($score > $max_score) {
		$max_score = $score;
		$winner = {path => \@path, map => dclone($map)};
		print "#" x 40, "\n$score : ", join(",",@path), "\n";

		for(my $i=0; $i < @task; $i++) {
			my ($start, $end) = @{$task[$i]};
			$map->draw_path(
				0,0,
				$path[$i]
			);
		}
	}
}
