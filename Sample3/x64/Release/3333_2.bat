for /l %%a in (1,1,100) do (
echo %%a
Sample3.exe config100_50.txt >> 100_50
Sample3.exe config200_60.txt >> 200_60
Sample3.exe config300_40.txt >> 300_40
Sample3.exe config400_50.txt >> 400_50
Sample3.exe config500_60.txt >> 500_60
Sample3.exe config600_40.txt >> 600_40
Sample3.exe config700_50.txt >> 700_50
Sample3.exe config800_60.txt >> 800_60
Sample3.exe config900_40.txt >> 900_40
Sample3.exe configA00_50.txt >> A00_50
)