run: mips_sim
	./mips_sim examples/square.hex

mips_sim: mips_sim.c
	dcc mips_sim.c -o mips_sim
