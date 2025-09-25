#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "data.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define RAND_RANGE(min, max) (rand() % ((max) - (min) + 1) + (min))

typedef struct territory
{
  char const *name;
  char const *color;
  int armies;
} territory_t;

static territory_t *create_territories(int count);
static void free_territories(territory_t *territories);
static void print_territories(territory_t *territories, int count);
static void attack(territory_t *attacker, territory_t *defender);
static void verify_mission_status(void);
static bool is_mission_completed(void);

typedef enum missions
{
  MISSION_CONQUER_COUNT = 0,
  MISSION_CONQUER_SEQUENCE = 1
} missions_e;

char const *MISSIONS[] = {
    "Conquer a specific number of territories.",
    "Conquer territories in sequence.",
};

missions_e current_mission = 0;
int conquer_count = 0;
int conquer_sequence = 0;
int required_conquer_count = 0;
int required_conquer_sequence = 0;

int main(void)
{
  srand(time(NULL));

  puts("Welcome to War!");

  int territory_count = RAND_RANGE(10, DATA_COUNT);
  territory_t *territories = create_territories(territory_count);
  if (!territories)
  {
    fputs("Failed to create territories.\n", stderr);
    return 1;
  }
  char const *player_color = territories[0].color;

  current_mission = RAND_RANGE(0, 1);
  char const *current_mission_desc = MISSIONS[current_mission];
  required_conquer_count = RAND_RANGE(3, territory_count / 2);
  required_conquer_sequence = RAND_RANGE(2, territory_count / 3);

  int op = 0;
  do
  {
    printf("Your team color is %s\n", player_color);
    print_territories(territories, territory_count);

    puts("Choose an operation:\n\t1. Attack\n\t2. Verify missions\n\t0. Exit");
    putchar('>');
    scanf("%d", &op);

    switch (op)
    {
    case 1:
    {
      int attacker_index = 0;
      int defender_index = -1;
      puts("Enter your territory index to attack from:");
      scanf("%d", &attacker_index);

      if (attacker_index < 0 || attacker_index >= territory_count)
      {
        fputs("Invalid territory index.\n", stderr);
        break;
      }

      if (strcmp(territories[attacker_index].color, player_color) != 0)
      {
        fputs("You can only attack from your own territories.\n", stderr);
        break;
      }

      puts("Enter territory index to attack:");
      scanf("%d", &defender_index);

      if (defender_index < 0 || defender_index >= territory_count)
      {
        fputs("Invalid territory index.\n", stderr);
        break;
      }

      attack(&territories[attacker_index], &territories[defender_index]);
    }
    break;

    case 2:
      verify_mission_status();
      break;

    default:
      break;
    }

    if (is_mission_completed())
    {
      puts("Congratulations! You have completed your mission!");
      break;
    }
  } while (op != 0);

  free(territories);

  return 0;
}

static void shuffle_data(char const **data, int count)
{
  for (int i = count - 1; i > 0; i--)
  {
    int j = rand() % (i + 1);
    char const *tmp = data[i];
    data[i] = data[j];
    data[j] = tmp;
  }
}

static territory_t *create_territories(int count)
{
  if (count <= 5 || count > DATA_COUNT)
    return NULL;

  territory_t *territories = malloc(count * sizeof(territory_t));
  if (!territories)
  {
    perror("Failed to allocate memory for territories");
    return NULL;
  }

  shuffle_data(countries, DATA_COUNT);
  shuffle_data(colors, DATA_COUNT);

  for (int i = 0; i < count; i++)
  {
    territories[i].name = countries[i];
    territories[i].color = colors[i];
    territories[i].armies = RAND_RANGE(100, 200);
  }

  return territories;
}

static void free_territories(territory_t *territories)
{
  if (!territories)
    return;

  free(territories);
}

static void print_territories(territory_t *territories, int count)
{
  puts("Territories:");
  printf("%6s %-35s %-20s %6s\n", "#", "Name", "Color", "Armies");
  for (int i = 0; i < count; i++)
  {
    printf("%6d %-35s %-20s %6d\n", i, territories[i].name, territories[i].color, territories[i].armies);
  }
}

static void attack(territory_t *attacker, territory_t *defender)
{
  if (!attacker || !defender)
  {
    fputs("Invalid territories for attack.\n", stderr);
    return;
  }

  if (attacker == defender)
  {
    fputs("Cannot attack the same territory.\n", stderr);
    return;
  }

  if (strcmp(attacker->color, defender->color) == 0)
  {
    fputs("Cannot attack conquered territories.\n", stderr);
    return;
  }

  if (attacker->armies <= 1)
  {
    fprintf(stderr, "%s must have more than 1 army to attack.\n", attacker->name);
    return;
  }

  int attacker_luck = RAND_RANGE(1, 6);
  int defender_luck = RAND_RANGE(1, 6);

  printf("%s rolls a %d\n", attacker->name, attacker_luck);
  printf("%s rolls a %d\n", defender->name, defender_luck);

  if (attacker_luck >= defender_luck)
  {
    printf("%s wins the attack against %s!\n", attacker->name, defender->name);
    defender->color = attacker->color;
    defender->armies = MAX(attacker->armies / 2, 1);
    attacker->armies -= defender->armies;

    conquer_count++;
    conquer_sequence++;
  }
  else
  {
    printf("%s defends successfully against %s!\n", defender->name, attacker->name);
    if (attacker->armies > 0)
      attacker->armies--;

    conquer_sequence = 0;
  }
}

static void verify_mission_status(void)
{
  switch (current_mission)
  {
  case MISSION_CONQUER_COUNT:
    printf("Conquered %d out of %d required territories.\n", conquer_count, required_conquer_count);
    break;

  case MISSION_CONQUER_SEQUENCE:
    printf("Current conquer sequence: %d out of %d required.\n", conquer_sequence, required_conquer_sequence);
    break;

  default:
    break;
  }
}

static bool is_mission_completed(void)
{
  switch (current_mission)
  {
  case MISSION_CONQUER_COUNT:
    return conquer_count >= required_conquer_count;

  case MISSION_CONQUER_SEQUENCE:
    return conquer_sequence >= required_conquer_sequence;

  default:
    return false;
  }
}
