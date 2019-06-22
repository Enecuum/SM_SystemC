for /l %%a in (1,1,100) do (
echo %%a
Sample3.exe config100_60.txt >> 100_60
Sample3.exe config200_40.txt >> 200_40
Sample3.exe config300_50.txt >> 300_50
Sample3.exe config400_60.txt >> 400_60
Sample3.exe config500_40.txt >> 500_40
Sample3.exe config600_50.txt >> 600_50
Sample3.exe config700_60.txt >> 700_60
Sample3.exe config800_40.txt >> 800_40
Sample3.exe config900_50.txt >> 900_50
Sample3.exe configA00_60.txt >> A00_60
)