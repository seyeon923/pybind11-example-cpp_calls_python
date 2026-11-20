import numpy as np


def check_3rd_parties():
    try:
        import cv2
        import matplotlib
        import matplotlib.pyplot as plt
        import numpy as np

        print(f"Numpy: {np.__version__}")
        print(f"OpenCV: {cv2.__version__}")
        print(f"Matplotlib: {matplotlib.__version__}")

        y, x = np.indices((1000, 1000))
        checkerboard = ((x // 100 + y // 100) % 2 * 255).astype(np.uint8)

        plt.imshow(checkerboard, cmap="gray")
        plt.show()

        return True
    except ImportError:
        return False


def pass_array(arr: np.ndarray):
    print("\n------ In Python pass_numpy_array ------")
    print("Passed array:")
    print(arr)
    print(f"    shape: {arr.shape}, dtype: {arr.dtype}")
    print("Flags:\n")
    print(arr.flags)

    arr *= 2
    print("Doubled the passed array!")
    print(f"Array after doubling:\n{arr}")

    print("----------------------------------------\n")


def add(i, j):
    return i + j
