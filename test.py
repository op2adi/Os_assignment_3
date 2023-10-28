import numpy as np
import matplotlib.pyplot as plt

def compute_fourier_series(N1, N):
    n = np.arange(-N1, N1 + 1)
    x = np.where(np.abs(n) <= N1, 1, 0) # isse signal defn di hai
    X = []
    for k in range(N):
        X_k = 0
        real,imag =[],[]
        for n_idx in range(-N1 ,N1+1):
            real.append(np.cos(k*n_idx*2*np.pi/N))
            imag.append(np.sin(k*n_idx*2*np.pi/N))
            # print(real,imag,k)
        # X_k+=real
        # X_k+=1j*imag
        # print(X_k)
        # X.append(X_k)
        # print(real)  # sb debugging
        # print(imag)
        print((sum(real)+sum(imag))/N)
        X.append((sum(real)+sum(imag))/N+0j)
    # Normalize the coefficients
    X = np.array(X)
    # print(X)

    magnitude = np.array(X)
    phase = np.angle(X)


    return X,phase

N1 = 2
N_a = 4 * N1 + 1
N_b = 8 * N1 + 1
N_c = 10 * N1 + 1

magnitude_a, phase_a = compute_fourier_series(N1, N_a)
magnitude_b, phase_b = compute_fourier_series(N1, N_b)
magnitude_c, phase_c = compute_fourier_series(N1, N_c)

# # Print the results
print("Case (a) Magnitude:", magnitude_a)
# print("Case (a) Phase:", phase_a)
print("Case (b) Magnitude:", magnitude_b)
# print("Case (b) Phase:", phase_b)
print("Case (c) Magnitude:", magnitude_c)
# print("Case (c) Phase:", phase_c)
def plot_fourier_series(X, phase, N, N1, case):
    frequencies = np.arange(N) #

    plt.figure(figsize=(12, 4))

    plt.subplot(1, 2, 1)
    plt.stem(frequencies, np.array(X), use_line_collection=True)
    plt.title(f'Case {case} Magnitude Spectrum (N={N}, N1={N1})')
    plt.xlabel('Frequency')
    plt.ylabel('Magnitude')
    plt.xticks(np.arange(0,N+1))
    plt.grid(True)

    # plt.subplot(1, 2, 2)
    # plt.stem(frequencies, phase, use_line_collection=True)
    # plt.title(f'Case {case} Phase Spectrum (N={N}, N1={N1})')  # yeh ji hai bekar nakara function kuch nhi krta
    # plt.xlabel('Frequency')
    # plt.ylabel('Phase (radians)')
    # plt.grid(True)

    plt.tight_layout()
    plt.show()

# Example usage:
X_a, phase_a = compute_fourier_series(N1, N_a)
X_b, phase_b = compute_fourier_series(N1, N_b)
X_c, phase_c = compute_fourier_series(N1, N_c)

plot_fourier_series(X_a, phase_a, N_a, N1, '(a)')
plot_fourier_series(X_b, phase_b, N_b, N1, '(b)')
plot_fourier_series(X_c, phase_c, N_c, N1, '(c)')

