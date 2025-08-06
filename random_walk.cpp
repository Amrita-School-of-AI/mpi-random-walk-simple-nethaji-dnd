#include <iostream>
#include <cstdlib> // For atoi, rand, srand
#include <ctime>   // For time
#include <mpi.h>

void walker_process();
void controller_process();

int domain_size;
int max_steps;
int world_rank;
int world_size;

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc != 3)
    {
        if (world_rank == 0)
        {
            std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <domain_size> <max_steps>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    domain_size = atoi(argv[1]);
    max_steps = atoi(argv[2]);

    if (world_rank == 0)
    {
        controller_process();
    }
    else
    {
        walker_process();
    }

    MPI_Finalize();
    return 0;
}

void walker_process()
{
    srand(time(NULL) + world_rank);  // Seed with time + rank for randomness
    int position = 0;
    int steps = 0;

    for (steps = 1; steps <= max_steps; ++steps)
    {
        int direction = (rand() % 2 == 0) ? -1 : 1;
        position += direction;

        if (position < -domain_size || position > domain_size)
        {
            std::cout << "Rank " << world_rank << ": Walker finished in " << steps << " steps (out of bounds)." << std::endl;

            int msg = steps;
            MPI_Send(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            break;
        }
    }

    if (steps > max_steps)
    {
        std::cout << "Rank " << world_rank << ": Walker finished in " << max_steps << " steps (max steps reached)." << std::endl;

        int msg = max_steps;
        MPI_Send(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
}

void controller_process()
{
    int walkers = world_size - 1;
    int msg;
    MPI_Status status;

    for (int i = 0; i < walkers; ++i)
    {
        MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        std::cout << "Controller: Received completion from Rank " << status.MPI_SOURCE << " in " << msg << " steps." << std::endl;
    }

    std::cout << "Controller: All " << walkers << " walkers have finished." << std::endl;
}
