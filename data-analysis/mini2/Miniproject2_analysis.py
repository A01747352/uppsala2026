"""
Miniproject 2: Classification of Handwritten Digits using SVD
================================================================

Implements and evaluates the SVD-based nearest-subspace classifier described
in the miniproject handout:

  1. For digits 3 and 8, compute the SVD of the (784 x 400) training matrix,
     plot the singular values on linear axes, and plot the first three
     singular images (left singular vectors reshaped to 28x28).

  2. Implement the full SVD-based classification algorithm: for every digit
     0-9, build a (784 x 400) training matrix, compute its SVD, and keep the
     first k left singular vectors U_k as an orthogonal basis ("approximation
     space") for that digit's subspace. Classify a test image by picking the
     digit whose basis gives the smallest residual ||(I - U_k U_k^T) delta||_2,
     for k = 5, ..., 15. Evaluate on the full 40000-image test set.

Data files (from HandwrittenDigits.zip):
  TrainDigits.npy  (784, 240000) uint8  -- flattened 28x28 training images
  TrainLabels.npy  (1, 240000)   uint8  -- digit label (0-9) per training image
  TestDigits.npy   (784, 40000)  uint8  -- flattened 28x28 test images
  TestLabels.npy   (1, 40000)    uint8  -- digit label (0-9) per test image
"""

import os
import time

import matplotlib
import numpy as np

matplotlib.use("Agg")
import matplotlib.pyplot as plt

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(SCRIPT_DIR, "HandwrittenDigits")
FIG_DIR = os.path.join(SCRIPT_DIR, "figs")
N_TRAIN_PER_DIGIT = 400  # images used per digit to build each A_d
K_MAX = 15  # largest basis size we need
K_VALUES = list(range(5, 16))  # k = 5, 6, ..., 15


# ----------------------------------------------------------------------
# Data loading helpers
# ----------------------------------------------------------------------
def load_data():
    TrainMat = np.load(os.path.join(DATA_DIR, "TrainDigits.npy")).astype(np.float64)
    TrainLab = np.load(os.path.join(DATA_DIR, "TrainLabels.npy"))[0]
    TestMat = np.load(os.path.join(DATA_DIR, "TestDigits.npy")).astype(np.float64)
    TestLab = np.load(os.path.join(DATA_DIR, "TestLabels.npy"))[0]
    return TrainMat, TrainLab, TestMat, TestLab


def training_matrix(TrainMat, TrainLab, digit, n=N_TRAIN_PER_DIGIT):
    """Return the (784 x n) training matrix A_d for one digit."""
    idx = TrainLab == digit
    Ad = TrainMat[:, idx]
    return Ad[:, :n]


def to_image(col):
    """Reshape a flattened 784-vector back into a 28x28 image (see eq. 0.1)."""
    return np.reshape(col, (28, 28)).T


# ----------------------------------------------------------------------
# Task 1: SVD of digits 3 and 8 -- singular values & singular images
# ----------------------------------------------------------------------
def task1_svd_digits_3_and_8(TrainMat, TrainLab):
    results = {}
    for digit in [3, 8]:
        Ad = training_matrix(TrainMat, TrainLab, digit)
        U, S, Vt = np.linalg.svd(Ad, full_matrices=False)
        results[digit] = {"Ad": Ad, "U": U, "S": S, "Vt": Vt}
        print(
            f"Digit {digit}: A_{digit} shape = {Ad.shape}, "
            f"{len(S)} singular values, sigma_1={S[0]:.2f}, sigma_400={S[-1]:.2f}"
        )

        # --- singular value plot (linear scales) ---
        plt.figure(figsize=(6.5, 5))
        plt.plot(np.arange(1, len(S) + 1), S, marker="o", markersize=2.5, lw=1)
        plt.xlabel("Index j")
        plt.ylabel(r"Singular value $\sigma_j$")
        plt.title(f"Singular values of training matrix $A_{{{digit}}}$ (digit {digit})")
        plt.tight_layout()
        plt.savefig(f"{FIG_DIR}/singvals_digit{digit}.png", dpi=150)
        plt.close()

        # --- first three singular images ---
        fig, axes = plt.subplots(1, 3, figsize=(9, 3.2))
        for j in range(3):
            img = to_image(U[:, j])
            axes[j].imshow(img, cmap="gray")
            axes[j].set_title(f"$u_{{{j + 1}}}$")
            axes[j].axis("off")
        fig.suptitle(f"First three singular images, digit {digit}")
        plt.tight_layout()
        plt.savefig(f"{FIG_DIR}/singimages_digit{digit}.png", dpi=150)
        plt.close()
    return results


# ----------------------------------------------------------------------
# Task 2: full SVD-based classifier, evaluated for k = 5, ..., 15
# ----------------------------------------------------------------------
def precompute_bases(TrainMat, TrainLab, k_max=K_MAX, n=N_TRAIN_PER_DIGIT):
    """Compute and store U_kmax for each digit 0-9 (step 2: Training)."""
    bases = {}
    for digit in range(10):
        Ad = training_matrix(TrainMat, TrainLab, digit, n=n)
        U, S, Vt = np.linalg.svd(Ad, full_matrices=False)
        bases[digit] = U[:, :k_max]  # keep only the columns we will ever need
        print(
            f"Trained digit {digit}: A_{digit} shape {Ad.shape}, "
            f"stored U[:, :{k_max}] with shape {bases[digit].shape}"
        )
    return bases


def classify_all(bases, TestMat, k):
    """
    Step 3: Classification. For every test image, compute the residual
    ||(I - U_k U_k^T) delta||_2 in every digit's basis (vectorised over all
    40000 test images at once, see tip 4), and pick the digit with the
    smallest residual (tip 5).
    """
    n_test = TestMat.shape[1]
    residuals = np.empty((10, n_test))
    for digit in range(10):
        Uk = bases[digit][:, :k]  # (784, k)
        proj = Uk @ (Uk.T @ TestMat)  # (784, n_test) projection onto the subspace
        residuals[digit, :] = np.linalg.norm(TestMat - proj, axis=0)
    predictions = np.argmin(residuals, axis=0)
    return predictions


def evaluate(predictions, TestLab):
    """Overall and per-digit success rates (%)."""
    overall = 100.0 * np.mean(predictions == TestLab)
    per_digit = np.zeros(10)
    for digit in range(10):
        mask = TestLab == digit
        per_digit[digit] = 100.0 * np.mean(predictions[mask] == digit)
    return overall, per_digit


def task2_classification(TrainMat, TrainLab, TestMat, TestLab):
    print("\n--- Training: computing 10 SVDs (n=400 images each) ---")
    t0 = time.time()
    bases = precompute_bases(TrainMat, TrainLab)
    print(f"Training took {time.time() - t0:.2f} s")

    print("\n--- Classification: evaluating k = 5..15 on all 40000 test images ---")
    overall_rates = []
    per_digit_rates = np.zeros((len(K_VALUES), 10))
    t0 = time.time()
    for i, k in enumerate(K_VALUES):
        preds = classify_all(bases, TestMat, k)
        overall, per_digit = evaluate(preds, TestLab)
        overall_rates.append(overall)
        per_digit_rates[i, :] = per_digit
        print(
            f"k={k:2d}  overall success = {overall:5.2f}%   "
            f"per-digit = {np.array2string(per_digit, precision=1)}"
        )
    print(
        f"Classification of {TestMat.shape[1]} test images x {len(K_VALUES)} k-values "
        f"took {time.time() - t0:.2f} s"
    )

    # --- Figure reproducing the style of Figure 3 in the handout ---
    plt.figure(figsize=(7.5, 5.5))
    for digit in range(10):
        plt.plot(
            K_VALUES,
            per_digit_rates[:, digit],
            marker="o",
            markersize=3,
            label=str(digit),
        )
    plt.xlabel("k (columns of U)")
    plt.ylabel("Percentage of success")
    plt.title("Per-digit classification success vs. number of basis vectors k")
    plt.legend(title="Digit", bbox_to_anchor=(1.02, 1), loc="upper left")
    plt.tight_layout()
    plt.savefig(f"{FIG_DIR}/success_vs_k.png", dpi=150)
    plt.close()

    # --- Overall success rate vs k ---
    plt.figure(figsize=(6.5, 5))
    plt.plot(K_VALUES, overall_rates, marker="o", color="tab:blue")
    plt.xlabel("k (columns of U)")
    plt.ylabel("Overall percentage of success")
    plt.title("Overall classification success vs. k")
    plt.ylim(min(overall_rates) - 1, 100)
    plt.tight_layout()
    plt.savefig(f"{FIG_DIR}/overall_success_vs_k.png", dpi=150)
    plt.close()

    # --- Confusion matrix at the best k (for discussion) ---
    best_k = K_VALUES[int(np.argmax(overall_rates))]
    preds_best = classify_all(bases, TestMat, best_k)
    confusion = np.zeros((10, 10), dtype=int)
    for true_d in range(10):
        mask = TestLab == true_d
        for pred_d in range(10):
            confusion[true_d, pred_d] = np.sum(preds_best[mask] == pred_d)

    plt.figure(figsize=(6.5, 5.5))
    im = plt.imshow(confusion, cmap="Blues")
    plt.colorbar(im, label="Count")
    plt.xticks(range(10))
    plt.yticks(range(10))
    plt.xlabel("Predicted digit")
    plt.ylabel("True digit")
    plt.title(f"Confusion matrix (k = {best_k})")
    for i in range(10):
        for j in range(10):
            color = "white" if confusion[i, j] > confusion.max() / 2 else "black"
            plt.text(
                j, i, confusion[i, j], ha="center", va="center", color=color, fontsize=7
            )
    plt.tight_layout()
    plt.savefig(f"{FIG_DIR}/confusion_matrix.png", dpi=150)
    plt.close()

    # --- Example misclassified test images at k = best_k (for the appendix) ---
    wrong_idx = np.where(preds_best != TestLab)[0]
    rng = np.random.default_rng(0)
    chosen = rng.choice(wrong_idx, size=12, replace=False)
    fig, axes = plt.subplots(2, 6, figsize=(12, 5.2))
    for ax, idx in zip(axes.ravel(), chosen):
        img = to_image(TestMat[:, idx])
        ax.imshow(img, cmap="gray")
        ax.set_title(f"true {TestLab[idx]}, pred {preds_best[idx]}", fontsize=9)
        ax.axis("off")
    fig.suptitle(f"Examples of misclassified test images (k = {best_k})")
    plt.tight_layout(rect=[0, 0, 1, 0.94])
    plt.subplots_adjust(hspace=0.5)
    plt.savefig(f"{FIG_DIR}/misclassified_examples.png", dpi=150)
    plt.close()

    # --- Singular values on a log scale, digits 3 and 8 (bonus view for appendix) ---
    plt.figure(figsize=(6.5, 5))
    for digit, color in [(3, "tab:blue"), (8, "tab:orange")]:
        Ad = training_matrix(TrainMat, TrainLab, digit)
        S = np.linalg.svd(Ad, compute_uv=False)
        plt.semilogy(np.arange(1, len(S) + 1), S, label=f"digit {digit}", color=color)
    plt.xlabel("Index j")
    plt.ylabel(r"Singular value $\sigma_j$ (log scale)")
    plt.title("Singular values of $A_3$ and $A_8$ on a log scale")
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"{FIG_DIR}/singvals_logscale.png", dpi=150)
    plt.close()

    return {
        "overall_rates": overall_rates,
        "per_digit_rates": per_digit_rates,
        "best_k": best_k,
        "confusion": confusion,
    }


if __name__ == "__main__":
    os.makedirs(FIG_DIR, exist_ok=True)

    print("Loading data...")
    TrainMat, TrainLab, TestMat, TestLab = load_data()
    print(f"TrainDigits: {TrainMat.shape}, TrainLabels: {TrainLab.shape}")
    print(f"TestDigits:  {TestMat.shape}, TestLabels:  {TestLab.shape}")

    print("\n===== Task 1: SVD of digits 3 and 8 =====")
    svd38 = task1_svd_digits_3_and_8(TrainMat, TrainLab)

    print("\n===== Task 2: SVD-based classification =====")
    results2 = task2_classification(TrainMat, TrainLab, TestMat, TestLab)

    print("\n\n===== Summary =====")
    print(f"{'k':>3} | {'overall %':>10}")
    for k, rate in zip(K_VALUES, results2["overall_rates"]):
        print(f"{k:>3} | {rate:>10.2f}")
    print(
        f"\nBest k = {results2['best_k']}, "
        f"overall success = {max(results2['overall_rates']):.2f}%"
    )
