import random

def write_random_rational_pairs(filename):
    with open(filename, 'wb') as f:
        for _ in range(5):
            # Generate two random rational numbers
            num1 = random.uniform(-100., 100)
            num2 = random.uniform(-100., 100)
            # Write to file with required format
            f.write(f"COEFF {num1:.7f} {num2:.7f}".encode('ascii'))
            f.write("\r".encode('ascii'))
            f.write("\n".encode('ascii'))
write_random_rational_pairs("coeffs.txt")