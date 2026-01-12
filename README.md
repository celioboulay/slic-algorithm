# SLIC Superpixels

Implementation of the Simple Linear Iterative Clustering superpixel algorithm in C++ (https://www.epfl.ch/labs/ivrl/research/slic-superpixels/).


Dependencies:
- C++17
- OpenCV is used for image I/O and color space conversion.  
- Eigen is used for dense grids (labels / distances).

---

## Project structure

```
.
├─ include/
├─ src/        
├─ data/       
├─ build/          
└─ CMakeLists.txt
```

- `core.*` : SLIC algorithm
- `render.*` : visualization / rendering
- `io.*` : image loading and preprocessing
- `utils.*` : helper functions
- `main.cpp` : main