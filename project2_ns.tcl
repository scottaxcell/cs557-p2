set ns [new Simulator]
source tb_compat.tcl


# create nodes
set nodeA [$ns node]
tb-set-node-os $nodeA U1404-64-csu557aj

set node1 [$ns node]
tb-set-node-os $node1 U1404-64-csu557aj

set node2 [$ns node]
tb-set-node-os $node2 U1404-64-csu557aj

set node3 [$ns node]
tb-set-node-os $node3 U1404-64-csu557aj

set router0 [$ns node]
tb-set-node-os $router0 U1404-64-csu557aj

# create lan
set lan0 [$ns make-lan "$router0 $node1 $node2 $node3" 100Mb 500ms]
tb-set-node-lan-delay $node1 $lan0 500ms
tb-set-node-lan-delay $node2 $lan0 500ms
tb-set-node-lan-delay $node3 $lan0 500ms

# connect nodeA and router0
$ns duplex-link $nodeA $router0 100Mb 500ms DropTail
#tb-set-node-lan-delay $nodeA $router0 500ms

# run
$ns run

