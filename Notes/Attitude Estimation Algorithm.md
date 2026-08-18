# Coordinate System Definition
Due to the fixed installation of MPU6050, a body coordinate system is determined as follow:
![[c06cc817cef4c7cfd7c779e4925145de.jpg]]
$$
\begin{align}
\begin{cases}
\dot{\phi} = p + \left(r\cos\phi + q\sin\phi\right)\tan\theta \\
\dot{\theta} = q\cos\phi - r\sin\phi \\
\dot{\psi} = \dfrac{1}{\cos\theta}\left(r\cos\phi + q\sin\phi\right)
\end{cases} 
\end{align}
$$
$p,q,r$ are three angular velocity orbiting around three axis of the body coordinate system and they could be measured by MPU6050 directly(designated as $gx, gy, gz$ in code).

In this project, mechanical design of the car results in zero-pitch/theta. Thus $\dot{\phi}=p$, and $\dot{\psi} = r\cos \phi + q\sin \phi$.

If $\phi$ is very small, the latter could also be simplified as $\dot{\psi}=r$. However, it is not as applicable as the former. A better way to measure yaw is to use the encoders attached to the wheel.

The emphasis of this project is on keeping roll/$\phi$ around zero.

# $\phi, \theta$ Measured by Accelerometer
Assuming $\vec{a} = 0$, the output of the accelerometer, $\vec{f}$ equals to $-\vec{g}$。
Therefore, in the ground frame, $\vec{f}$ could be written as $[0\quad 0 \quad g]^T$ . While in the body frame, $\vec{f}$ should be written as:
$$
\begin{align}
\vec{f} = \left(R_z(\psi)R_y(\theta)R_x(\phi) \right)^{-1}\begin{bmatrix}
0\\
0\\
g
\end{bmatrix}=\begin{bmatrix}
-g\sin\theta \\
g\cos\theta\sin\phi \\
g\cos\theta\cos\phi
\end{bmatrix}
\end{align}
$$
Therefore, $\phi = \arctan_{2}{\frac{ay}{az}}$ and $\theta = \arcsin\left( \frac{ax}{g} \right)$. or $\theta = \arctan\left( \frac{ax}{\sqrt{ ay^2 + az^2 }} \right)$, which is more stable because arctan doesn't have a limitation on the domain of definition. 
(Note: the primary values of three angles are limited as $\phi \in (-\pi,\pi],\theta \in \left[ -\frac{\pi}{2}, \frac{\pi}{2} \right], \psi \in  (-\pi,\pi].$)

 
# Roll-Measurement: Complementary Filter
For an accelerometer, the output $\vec{f}=\vec{a}-\vec{g}$. Only when $\vec{a}=0$, can $\phi$ be calculated as $\phi = \arctan_{2}{\frac{ay}{az}}$. Actually non-zero $\vec{a}$ is caused by vibration, sudden launch and stop, which get high-frequency noise included; For a gyroscope, the static bias and errors accumulated gradually in every linear approximation step both act as low-frequency noise. Therefore, by applying a low-pass filter to the accelerometer and a high-pass filter to the gyroscope, then adding them together, we can gain a ideal measurement for $\phi$.
## LPF & HPF (one order)

$$
\begin{align}
LPF: H_{L}(s) = \frac{1}{\tau s+1} \\
HPF: H_{H}(s) = \frac{\tau s}{\tau s+1} \\
H_{L}(s) + H_{H}(s) = 1
\end{align}
$$
![[Pasted image 20260811202615.png]]

## Theoretical Derivation
The basic idea is $\hat{\Phi}(s) = H_L(s)\Phi_a(s) + H_H(s)\Phi_g(s)$, where $\Phi_a(s)$ comes from the accelerometer, $\Phi_g(s)$ comes from the gyroscope and $\hat{\Phi}(s)$ is the estimated roll angle.

Expand the basic idea with expressions of LPF&HPF:
$$
\begin{align}
\hat{\Phi}(s) &= \frac{1}{\tau s+1}\Phi_a(s)
+ \frac{\tau s}{\tau s+1}\Phi_g(s)
\end{align}
$$
where $\Phi_{g}(s) = \frac{\Omega_{g}(s)}{s}$, due to $\phi=\int_{0}^t\omega_{g}(\tau)d\tau$. Furthermore,
$$
\begin{align}
\tau s\hat{\Phi}(s)+\hat{\Phi}(s) &= \Phi_a(s)+\tau\Omega_g(s)
\end{align}
$$
Then, we apply $\mathcal{L}^{-1}$ to the equation above, to get the time-domain equation:
$$
\tau \frac{d\hat{\phi}}{dt} + \hat{\phi}
= \phi_a + \tau \omega_g
$$
Next, use backward difference $\frac{\mathrm{d}\hat{\phi}}{\mathrm{d}t} \approx \frac{\hat{\phi}[k]-\hat{\phi}[k-1]}{\Delta t}$ to replace derivative operation to change it into discrete form:
$$
\hat{\theta}[k]=\frac{\tau}{\tau+\Delta t}(\hat{\theta}[k-1]+\omega_g[k]\Delta t)+\frac{\Delta t}{\tau+\Delta t}\theta_a[k]
$$
(Sorted)
Define $\alpha=\frac{\tau}{\tau+\Delta t}$ to simplify this equation as:
$$
\hat{\theta}[k]=\alpha\left(\hat{\theta}[k-1]+\omega_g[k]\Delta t\right)+(1-\alpha)\theta_a[k]
$$

In this project, sampling time $\Delta t = 0.005s$ and $\tau = 0.1s$ (which means $\omega_{c}$ is specified as 10 rad/s), so that $\alpha= 0.95238$.

# Yaw-Measurement: Encoder Difference