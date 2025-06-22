<center><h3>图形学渲染大作业：路径追踪 实验报告</h3></center>

<center>吕先艨 2022013028</center>

<b>一、基础要求</b>

1. Whitted-Style 模型（反射、折射、阴影）

   下图是两个场景，包含两个点光源，其中右图包含反射和折射，正方体为折射材料：

<div style="text-align: center;">
  <img src="output\scene01_01.bmp" width="45%" />
  <img src="output\scene02_01.bmp" width="45%" />
</div>

​	下图是一个包含反射球和折射球的场景：

<img src="output\scene15_01.bmp" alt="scene15_01" style="zoom:50%;" />

2. Whitted-Style 路径追踪 对比（点光源改为面光源，路径追踪采用 Blinn-Phong BRDF 模型，多次镜面反射/折射/漫反射效果）

<div style="text-align: center;">
  <img src="output\scene02_01.bmp" width="45%" />
  <img src="output\scene03_06.bmp" width="45%" />
</div>
3. glossy BRDF

   实现了三种 glossy BRDF：Blinn-Phong BRDF，Cook-Torrance BRDF，Ward BRDF，其中 Ward BRDF 在加分项中说明；大部分场景采用 Blinn-Phong 模型，下图左侧球为 Blinn-Phong BRDF，右侧球为 Cook-Torrance BRDF，可看出 Cook-Torrance BRDF 更具有金属质感。

   <img src="output\scene08_03.bmp" alt="scene08_03" style="zoom:50%;" />

4. NEE

   下图左侧为球面均匀采样，右侧为 NEE 并用 MIS 结合的采样。可看出噪点明显减少：

<div style="text-align: center;">
  <img src="output\scene03_01.bmp" width="45%" />
  <img src="output\scene03_02.bmp" width="45%" />
</div>

使用 NEE 会有收敛速度的明显提升。这是因为某个面上一点受到光源直射的范围可能只占 $2\pi$ 角空间的很小一部分，然而却贡献了绝大多数的亮度，如果随机采样很难采样到这部分贡献。

<b>二、加分项</b>

实现了 7 个低加分项、2个中加分项。

低加分项：

1. MIS（cos-weighted 采样，光源采样，BRDF 采样）

   下图分别为：均匀采样 NEE，cos-weighted 采样 NEE，BRDF 采样 NEE，SPP 均为 1024

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene03_02.bmp" width="30%" />
     <img src="output\scene03_03.bmp" width="30%" />
     <img src="output\scene03_04.bmp" width="30%" />
   </div>
   
   
   局部放大图：
   
   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene03_02f.png" width="30%"/>
     <img src="output\scene03_03f.png" width="30%"/>
     <img src="output\scene03_04f.png" width="30%" />
   </div>

​	下图为将 cos-weighted 采样和 BRDF 采样用 MIS 结合的结果，噪点更少：

<img src="output\scene03_05.bmp" alt="scene03_05" style="zoom:50%;" />

​	下图是另一个场景下 cos-weighted NEE 和 MIS 的结果对比，SPP = 32：

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene08_01.bmp" width="40%" />
     <img src="output\scene08_02.bmp" width="40%" />
   </div>
2. 抗锯齿（用 Hammersley 序列在像素内随机采样方向；FXAA 作为图像后处理）

   下图为不采用 FXAA 的情况下，不用和用 Hammersley 序列的结果对比：

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene04_01.bmp" width="40%" />
     <img src="output\scene04_02.bmp" width="40%" />
   </div>
   下图为采用 FXAA 的情况下同样的两张图：
   
   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene04_03.bmp" width="40%" />
     <img src="output\scene04_04.bmp" width="40%" />
   </div>


   下图均采用 Hammersley 采样，但左图未采用 FXAA：

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene05_01.bmp" width="40%" />
     <img src="output\scene05_02.bmp" width="40%" />
   </div>
   局部放大图：

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene05_01f.png" width="40%" />
     <img src="output\scene05_02f.png" width="40%" />
   </div>
   FXAA 处理的区域：

<img src="output\diff.png" alt="diff" style="zoom:50%;" />

可看出 FXAA 准确勾勒出了场景中的边界。此外对于局部的噪点，FXAA 也有一定抑制作用。

Hammersley 采样和 FXAA 都具有一定的抗锯齿效果，二者叠加效果更好。大部分场景都同时采用了这两种方法。

3. 纹理贴图、法向贴图

   下图是一个球体的纹理贴图：

   <img src="output\scene06_01.bmp" alt="scene06" style="zoom:50%;" />

   下图是两个有纹理贴图和法向贴图的三角网格物体：

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene09_01.bmp" width="40%" />
     <img src="output\scene11_01.bmp" width="40%" />
   </div>

4. 伽马校正

   对三组场景，伽马校正前后的图像分别如下：

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene05_02.bmp" width="40%" />
     <img src="output\scene05_03.bmp" width="40%" />
   </div>

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene10_01.bmp" width="40%" />
     <img src="output\scene10_02.bmp" width="40%" />
   </div>

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene07_01.bmp" width="40%" />
     <img src="output\scene07_02.bmp" width="40%" />
   </div>


   可看出校正后的图像显得更加真实。

5. OpenMP 并行加速

   以下图为例：没有 OpenMP 的运行时间为 31.77s，有 OpenMP 的运行时间为 5.22s。效率大幅提升。

   <img src="output\scene08_02.bmp" alt="scene08_02" style="zoom:50%;" />

6. 各向异性 BRDF（Ward 模型）

   第一个场景是普通 Blinn-Phong BRDF 材料，第二个和第三个都是 Ward BRDF，其中第二个图中的各向异性沿 z 轴（视线方向），第三个图中沿轴向。漫反射概率均为 20%。概率的调整需要通过调节 BRDF 模型的 $\rho_d$ 和 $\rho_s$。 

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene14_01.bmp" width="30%" />
     <img src="output\scene14_02.bmp" width="30%" />
     <img src="output\scene14_03.bmp" width="30%" />  
   </div>

​	下图是漫反射概率为 60% 的情况，可看出材质更接近漫反射。

<img src="output\scene14_04.bmp" alt="scene14_04" style="zoom:50%;" />

7. 部分反射（菲涅尔系数）

   左图是完美折射材料，只发生全反射；右图在不发生全反射时按照 Schlick 近似确定反射率。右边球的折射率和玻璃相同。

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene15_02.bmp" width="40%" />
     <img src="output\scene15_03.bmp" width="40%" />  
   </div>
   下图左边的球折射率和水相同，右边的球折射率和玻璃相同，两个球的位置对称。可看出玻璃球的折射率更大（聚焦功能更强），并且反射率更高。二者都会发生全反射。
   
   <img src="output\scene16_01.bmp" alt="scene16_01" style="zoom:50%;" />
   
   下图中绿色地面上有一层很薄的水，可看出反射和折射效果，并且离地面越远的物体镜像越模糊，说明反射率随入射角度减小而减小。这和 Schlick 近似中的反射率 $F_0+\left(1-F_0\right)\left(1-\cos\theta_l\right)^5$ 是符合的。
   
   <img src="output\scene17_01.bmp" alt="scene17_01" style="zoom:50%;" />

中加分项：

1. 参数曲面解析法求交

   下图为两组场景，分别是 Bezier 曲线和 B 样条曲线，每组均包含三角网格近似的旋转体和解析法求交得到的旋转体两张图：

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene12_01.bmp" width="40%" />
     <img src="output\scene12_02.bmp" width="40%" />
   </div>

   <div style="display: flex; justify-content: center; gap: 10px;">
     <img src="output\scene13_01.bmp" width="40%" />
     <img src="output\scene13_02.bmp" width="40%" />
   </div>


   可看出基本没有色差，三角网格近似的旋转体具有明显的棱角，解析法求交的旋转体更加光滑。

2. 包围盒求交加速

   大部分包含复杂三角网格的场景基本没有办法在没有包围盒加速的情况下渲染。即使是比较简单的三角网格（如 NEE 中用到的正方体，8 个面 12 个三角形）加上包围盒也有巨大的性能提升。以下图（SPP = 16）为例，下图的正方形折射材料如果不加 BVH 加速则渲染时间为 60.40s，加上后为 38.35s。

   <img src="output\scene03_07.bmp" alt="scene03_07" style="zoom:50%;" />

<b>三、部分实现细节</b>

1. 绝大部分路径追踪所用的材质均为 Blinn-Phong BRDF 模型，采用的是一种归一化的 BRDF 形式，表达式为：
   $$
   f(l\to v)=\dfrac{\rho_d}{2\pi}+\rho_s\dfrac{\left(s+2\right)\left(s+4\right)}{8\pi\left(2^{-s/2}+s\right)}\left(\cos\theta_h\right)^s
   $$
   归一化系数参考自 https://www.farbrausch.de/~fg/articles/phong.pdf；

2. 对于理想折射材料的情况，假设入射角 $\theta_i$，出射角 $\theta_o$，光线损失率为 $r$，由 BSDF 的能量守恒方程
   $$
   \int_{\Omega}f_r\left(w_i,w'\right)\cos\theta\mathrm{d}w'=1-r
   $$
   可知 $$f_r\left(w_i,w_o\right)=\dfrac{1-r}{\cos\theta_o}$$，由渲染方程还需要乘上 $\cos\theta_i$ 的系数，因此在折射时光强需要乘上 $\left(1-r\right)\dfrac{\cos\theta_i}{\cos\theta_o}$；

3. 在计算纹理时，对于球和旋转体的纹理，可直接由交点坐标用公式计算 UV 值；对三角网格，采取的是先找到相交的三角形，再计算三角形三点的权重，加权平均得到交点处的纹理值和法线方向；纹理用到了一些网络的图片，大部分是 jpg 或 png 格式，因此使用了 stb_image 代码库来实现各种不同格式的图片读取；

4. 伽马校正的实现中，对于普通颜色不做校正，纹理输入时先取 $1/\gamma$ 次方，输出时对像素取 $\gamma$ 次方，来保持纹理的图像不变；

5. OpenMP 的实现中，通过 collapse(2) 展开两层循环，将像素分配到线程上，并且用 scheduled(guided) 来防止不同像素计算时的速度不均衡导致的负载不均衡，最大程度地利用各个线程；

6. 在多重重要性采样（MIS）的实现中，实现了 cos-weighted 采样 (1)、BRDF 采样 (2)、光源采样 (3) 三种采样方式，其中 1/3 和 2/3 都可以直接结合，因为光源采样不需要继续递归；而 1/2 不能直接结合，因为一次 SPP 只能计算一条光路。为了同时实现三种采样，采用了每次等概率选择1、2 中的一种采样的方法。三种采样方式参考自三篇知乎文章：https://zhuanlan.zhihu.com/p/503163354，https://zhuanlan.zhihu.com/p/505284731，https://zhuanlan.zhihu.com/p/508136071；Blinn-Phong BRDF 模型的采样方式参考了 https://zhuanlan.zhihu.com/p/58205525；

7. 在参数曲面解析法求交中，先建立了足够密的三角网格来近似曲面，作为牛顿迭代法的初值。这样做有时会发生曲面上发出的光线落在三角网格内部的情况，牛顿迭代会求出自交的结果，进而被错误判定为没有交点。因此，在没有交点时算法会寻找光线与三角网格的下一个交点，再次作为牛顿迭代的初值，直到找到交点为止。经过这样的调整，牛顿迭代法的图像与三角网格的图像除了棱角处之外基本没有任何区别；

8. 在包围盒求交加速中，采取的是一种较为简单的划分方法：先建立最大的包围盒，每次切分包围盒三个维度中最长的一维，将各个三角形面片按这一维的三个点最小值排序，然后将所有三角形分为两部分，变为两个子包围盒，形成二叉树结构；在求交时，先判断光线与包围盒有无交点，若存在交点且交点处的 t 不超过当前 hit 的 t，则根据光线与两个子包围盒相交的顺序递归求交；

9. 在实现的过程中，发现了很多物体求交时的正面或反面会对后续过程产生不同的效果：折射材料从内向外折射和从外向内折射有根本性的区别；旋转体可能只在正面有纹理，反面是材料本身的颜色。因此在 hit 类中加入了 <code>isFront</code> bool 变量表示是否打到材料的正面，并且对原本的一些求交代码做了改动，以支持反面的交点，如球体求交。然而这导致了精度问题：有的光线从球体外打到球体上，在球体上反射时由于精度误差起点已经到了球体内部，即使设置 <code>tmin = 1e-4</code> 对起点进行偏移也没有用，造成了图像上的部分噪点。这个问题是在实现菲涅尔折射材料时发现的，深入研究后发现主要的精度误差并不来自于求交的代码，而可能是由于 hit 的 t 是 float 类型导致有效数字不够。只将 t 改为 double 也是没有用的，因为 hit 类型不直接存储交点，而是通过 ray 类的 <code>pointAtParameter</code> 函数间接计算，而光线的起点和方向都是 float 类型，可能根本的解决办法是将模型的所有 float 类型换成 double。

