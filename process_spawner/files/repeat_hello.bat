@echo off

for /l %%x in (1, 1, 10) do (
	echo Hello number %%x
	ping 127.0.0.1 -n 2 > nul
)
