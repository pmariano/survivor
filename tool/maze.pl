use strict;
use warnings;

use Games::Maze;

my $m = Games::Maze->new(dimensions => [10,7,1]);

$m->make();
$m->solve();

print $m->to_ascii();

