for /l %%a in (1,1,100) do (
echo %%a
Sample3.exe config100_40.txt >> 100_40
Sample3.exe config200_50.txt >> 200_50
Sample3.exe config300_60.txt >> 300_60
Sample3.exe config400_40.txt >> 400_40
Sample3.exe config500_50.txt >> 500_50
Sample3.exe config600_60.txt >> 600_60
Sample3.exe config700_40.txt >> 700_40
Sample3.exe config800_50.txt >> 800_50
Sample3.exe config900_60.txt >> 900_60
Sample3.exe configA00_40.txt >> A00_40
)