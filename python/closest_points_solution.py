# Algorithm Design by Jon Kleinberg, Eva Tardos

"""
Given n points in the plane, find the pair that is closest together.
"""

if __name__ == "__main__":

    from random import sample
    import timeit


    def random_sample_test(size):
        x = sample(range(size*100), size)
        y = sample(range(size*100), size)
        return [Point(x,y) for x,y in list(zip(x, y))]

    def main():
        # P = file_test(sys.argv[1])
        P = random_sample_test(10000)
        print(f"nlogn_solution {timeit.timeit(lambda: nlogn_solution(P), number=1)}")
        print(f"nlogn_solution_par 4 {timeit.timeit(lambda: nlogn_solution_par(P, 4), number=1)}")
        print(f"nlogn_solution_par 8 {timeit.timeit(lambda: nlogn_solution_par(P, 8), number=1)}")

    main()
