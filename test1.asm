addi $t1,$zero,14
add $t2,$t1,$t1
andi $t3,$t1,7
sub $t4,$t2,$t3
ori $t5,$t4,1
sw $t5,(1024)$t3
lw $t6,(1024)$t3