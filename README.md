# thread-tests

Job applications am I right guys? (part 2)

## Standard procedure

```shell
git clone https://github.com/IS2511/thread-tests.git
cd thread-tests
mkdir build && cd build
cmake ..
make
```

## Example results

For better results (lol) the .tmp file was removed (if present) before starting

An old HDD via usb:
```c++
Starting threads...
Doing the thing................................................................................................................................................................................................................................................................
Program finished!
Queue sizes on each write: 1 19 33 46 60 73 85 96 106 116 126 136 146 156 167 177 187 197 208 218 228 235 234 233 232 231 230 229 228 227 226 225 224 223 222 221 220 219 218 217 216 215 214 213 212 211 210 209 208 207 206 205 204 203 202 201 200 199 198 197 196 195 194 193 192 191 190 189 188 187 186 185 184 183 182 181 180 179 178 177 176 175 174 173 172 171 170 169 168 167 166 165 164 163 162 161 160 159 158 157 156 155 154 153 152 151 150 149 148 147 146 145 144 143 142 141 140 139 138 137 136 135 134 133 132 131 130 129 128 127 126 125 124 123 122 121 120 119 118 117 116 115 114 113 112 111 110 109 108 107 106 105 104 103 102 101 100 99 98 97 96 95 94 93 92 91 90 89 88 87 86 85 84 83 82 81 80 79 78 77 76 75 74 73 72 71 70 69 68 67 66 65 64 63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 
Total time taken: 2.99338
Average time per write: 0.0116929
Average write MB/s: 85.522
```

Average HDD over SATA:
```c++
Starting threads...
Doing the thing................................................................................................................................................................................................................................................................
Program finished!
Queue sizes on each write: 1 1 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 3 3 3 3 3 3 3 3 3 3 3 3 3 4 5 6 7 8 9 10 11 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 12 13 13 13 13 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 14 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 
Total time taken: 0.219959
Average time per write: 0.000859213
Average write MB/s: 1163.86
```

Average SSD over SATA:
```c++
Starting threads...
Doing the thing................................................................................................................................................................................................................................................................
Program finished!
Queue sizes on each write: 1 1 2 3 4 4 4 5 6 7 8 9 9 10 11 11 12 13 14 15 16 16 16 16 16 16 16 16 16 16 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 
Total time taken: 0.219789
Average time per write: 0.00085855
Average write MB/s: 1164.75
```

### Summary

Not much difference between any SATA connected disk,
probably because of caching and small data batches (1MB)


Mildly interesting are patterns in `Queue sizes`:
- On a slow disk the pattern is easily explained, pushing to queue
  is much faster then writing to disk so we see a fast climb in numbers,
  followed by a slow decline at the rate of -1 (for each disk write)
- In the second result the climb is very slow until the end and the
  decline is seen only at the end
- The third result's data is a bit weird and I'm not sure why, but
  my understanding is caching and weird timing at the `wait_until()`
  are the main culprits
  
`Average write` results seem very inaccurate on faster disks because
of good caching and maybe semi-bad `chrono` timing
