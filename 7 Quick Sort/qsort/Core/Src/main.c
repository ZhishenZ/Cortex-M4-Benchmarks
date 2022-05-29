/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */


/* start measuring options ---------------------------------------------------------*/
#define STRING_100
#define USE_CCM
#define INPUT_CCM
/* end measuring options ---------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#define MAXARRAY 60000
#define MAX_STRING_SIZE 24


struct myStringStruct {
  char qstring[MAX_STRING_SIZE];
};

#ifdef USE_CCM
	__attribute__((section(".ccmram")))
#endif

int compare(const void *elem1, const void *elem2)
{
  int result;

  result = strcmp((*((struct myStringStruct *)elem1)).qstring, (*((struct myStringStruct *)elem2)).qstring);

  return (result < 0) ? 1 : ((result == 0) ? 0 : -1);
}

	#include <_ansi.h>
	#include <stdlib.h>

	#define _PARAMS(paramlist)		paramlist
	#define	_DEFUN(name, arglist, args)	name(args)
	#define	_DEFUN_VOID(name)		name(_NOARGS)
	#define	_AND		,

	#ifndef __GNUC__
	#define inline
	#endif
	static inline char	*med3 _PARAMS((char *, char *, char *, int (*)()));
	static inline void	 swapfunc _PARAMS((char *, char *, int, int));
	#define min(a, b)	(a) < (b) ? a : b
	/*
	 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
	 */
	#define swapcode(TYPE, parmi, parmj, n) { 		\
		long i = (n) / sizeof (TYPE); 			\
		register TYPE *pi = (TYPE *) (parmi); 		\
		register TYPE *pj = (TYPE *) (parmj); 		\
		do { 						\
			register TYPE	t = *pi;		\
			*pi++ = *pj;				\
			*pj++ = t;				\
	        } while (--i > 0);				\
	}
	#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(long) || \
		es % sizeof(long) ? 2 : es == sizeof(long)? 0 : 1;


	#ifdef USE_CCM
		__attribute__((section(".ccmram")))
	#endif
	static inline void
	_DEFUN(swapfunc, (a, b, n, swaptype),
		char *a _AND
		char *b _AND
		int n _AND
		int swaptype)
	{
		if(swaptype <= 1)
			swapcode(long, a, b, n)
		else
			swapcode(char, a, b, n)
	}
	#define swap(a, b)					\
		if (swaptype == 0) {				\
			long t = *(long *)(a);			\
			*(long *)(a) = *(long *)(b);		\
			*(long *)(b) = t;			\
		} else						\
			swapfunc(a, b, es, swaptype)
	#define vecswap(a, b, n) 	if ((n) > 0) swapfunc(a, b, n, swaptype)

	#ifdef USE_CCM
		__attribute__((section(".ccmram")))
	#endif
	static inline char *
	_DEFUN(med3, (a, b, c, cmp),
		char *a _AND
		char *b _AND
		char *c _AND
		int (*cmp)())
	{
		//int aaa = 0;

		return cmp(a, b) < 0 ?
		       (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a ))
	              :(cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c ));
	}

	#ifdef USE_CCM
		__attribute__((section(".ccmram")))
	#endif
	void
	_DEFUN(my_qsort, (a, n, es, cmp),
		void *a _AND
		size_t n _AND
		size_t es _AND
		int (*cmp)())
	{
//		int aaa = 0;
//		aaa++;
		char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
		int d, r, swaptype, swap_cnt;
	loop:	SWAPINIT(a, es);
		swap_cnt = 0;
		if (n < 7) {
			for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
				for (pl = pm; pl > (char *) a && cmp(pl - es, pl) > 0;
				     pl -= es)
					swap(pl, pl - es);
			return;
		}
		pm = (char *) a + (n / 2) * es;
		if (n > 7) {
			pl = a;
			pn = (char *) a + (n - 1) * es;
			if (n > 40) {
				d = (n / 8) * es;
				pl = med3(pl, pl + d, pl + 2 * d, cmp);
				pm = med3(pm - d, pm, pm + d, cmp);
				pn = med3(pn - 2 * d, pn - d, pn, cmp);
			}
			pm = med3(pl, pm, pn, cmp);
		}
		swap(a, pm);
		pa = pb = (char *) a + es;
		pc = pd = (char *) a + (n - 1) * es;
		for (;;) {
			while (pb <= pc && (r = cmp(pb, a)) <= 0) {
				if (r == 0) {
					swap_cnt = 1;
					swap(pa, pb);
					pa += es;
				}
				pb += es;
			}
			while (pb <= pc && (r = cmp(pc, a)) >= 0) {
				if (r == 0) {
					swap_cnt = 1;
					swap(pc, pd);
					pd -= es;
				}
				pc -= es;
			}
			if (pb > pc)
				break;
			swap(pb, pc);
			swap_cnt = 1;
			pb += es;
			pc -= es;
		}
		if (swap_cnt == 0) {  /* Switch to insertion sort */
			for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
				for (pl = pm; pl > (char *) a && cmp(pl - es, pl) > 0;
				     pl -= es)
					swap(pl, pl - es);
			return;
		}
		pn = (char *) a + n * es;
		r = min(pa - (char *)a, pb - pa);
		vecswap(a, pb - r, r);
		r = min(pd - pc, pn - pd - es);
		vecswap(pb, pn - r, r);
		if ((r = pb - pa) > es)
			my_qsort(a, r / es, es, cmp);
		if ((r = pd - pc) > es) {
			/* Iterate rather than recurse to save stack space */
			a = pn - r;
			n = r / es;
			goto loop;
		}
	/*		qsort(pn - r, r / es, es, cmp);*/
	}



#ifdef STRING_50
#ifdef INPUT_CCM
	__attribute__((section(".ccmram")))
#endif
	#define NUMBER_OF_STRING 50
	const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
	{
			"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis"
	};

#endif


#ifdef STRING_100
#ifdef INPUT_CCM
	__attribute__((section(".ccmramzz")))
#endif
	#define NUMBER_OF_STRING 100
	const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
	{

	"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
	"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of"

	};

#endif



#ifdef STRING_200
#ifdef INPUT_CCM
	__attribute__((section(".ccmram")))
#endif
	#define NUMBER_OF_STRING 200
	const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
	{

			"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
			"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of",
			"yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra",
			"equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts"

};

#endif



#ifdef STRING_300
#ifdef INPUT_CCM
	__attribute__((section(".ccmram")))
#endif
	#define NUMBER_OF_STRING 300
	const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
	{

		"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
		"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of",
		"yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra",
		"equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts",
		"Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me",
		"how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most"

};

#endif


#ifdef STRING_400
#ifdef INPUT_CCM
	__attribute__((section(".ccmram")))
#endif
	#define NUMBER_OF_STRING 400
	const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
	{

		"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
		"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of",
		"yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra",
		"equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts",
		"Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me",
		"how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most",
		"interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when", "theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary",
		"Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are", "everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own"

	};

#endif




#ifdef STRING_500
#ifdef INPUT_CCM
	__attribute__((section(".ccmram")))
#endif
	#define NUMBER_OF_STRING 500
	const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
	{

		"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
		"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of",
		"yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra",
		"equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts",
		"Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me",
		"how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most",
		"interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when", "theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary",
		"Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are", "everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own",
		"Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you", "dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice",
		"to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you", "in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older"
};
#endif



#ifdef STRING_750
#ifdef INPUT_CCM
	__attribute__((section(".ccmram")))
#endif
	#define NUMBER_OF_STRING 750
	const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
	{

		"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
		"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of",
		"yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra",
		"equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts",
		"Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me",
		"how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most",
		"interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when", "theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary",
		"Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are", "everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own",
		"Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you", "dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice",
		"to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you", "in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older",
		"you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York", "City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes", "you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will",
		"philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were", "reasonable", "politicians", "were", "noble", "and", "children", "respected", "their", "elders", "Respect", "your", "elders", "Dont", "expect", "anyone", "else", "to", "support", "you", "Maybe", "you", "have", "a", "trust", "fund", "Maybe", "youll", "have", "a", "wealthy", "spouse",
		"But", "you", "never", "know", "when", "either", "one", "might", "run", "out", "Dont", "mess", "too", "much", "with", "your", "hair", "or", "by", "the", "time", "youre", "40", "it", "will", "look", "85", "Be", "careful", "whose", "advice", "you", "buy", "but", "be", "patient", "with", "those", "who", "supply", "it", "Advice", "is", "a", "form", "of", "nostalgia", "Dispensing", "it", "is",
		"a", "way", "of", "fishing", "the", "past", "from", "the", "disposal", "wiping", "it", "off", "painting", "over", "the", "ugly", "parts", "and", "recycling", "it", "for", "more", "than", "its", "worth", "But", "trust", "me", "on", "the", "sunscreen", "Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could",
		"offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis", "more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of"
};
#endif



#ifdef STRING_1000
#ifdef INPUT_CCM
	__attribute__((section(".ccmram")))
#endif
	#define NUMBER_OF_STRING 1000
	const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
	{

		"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
		"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of",
		"yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra",
		"equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts",
		"Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me",
		"how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most",
		"interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when", "theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary",
		"Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are", "everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own",
		"Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you", "dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice",
		"to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you", "in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older",
		"you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York", "City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes", "you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will",
		"philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were", "reasonable", "politicians", "were", "noble", "and", "children", "respected", "their", "elders", "Respect", "your", "elders", "Dont", "expect", "anyone", "else", "to", "support", "you", "Maybe", "you", "have", "a", "trust", "fund", "Maybe", "youll", "have", "a", "wealthy", "spouse",
		"But", "you", "never", "know", "when", "either", "one", "might", "run", "out", "Dont", "mess", "too", "much", "with", "your", "hair", "or", "by", "the", "time", "youre", "40", "it", "will", "look", "85", "Be", "careful", "whose", "advice", "you", "buy", "but", "be", "patient", "with", "those", "who", "supply", "it", "Advice", "is", "a", "form", "of", "nostalgia", "Dispensing", "it", "is",
		"a", "way", "of", "fishing", "the", "past", "from", "the", "disposal", "wiping", "it", "off", "painting", "over", "the", "ugly", "parts", "and", "recycling", "it", "for", "more", "than", "its", "worth", "But", "trust", "me", "on", "the", "sunscreen", "Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could",
		"offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis", "more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of",
		"your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of", "yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous",
		"you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra", "equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed",
		"your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts", "Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre",
		"ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me", "how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont",
		"know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most", "interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when"
};
#endif



#ifdef STRING_1250
#ifdef INPUT_CCM
	__attribute__((section(".ccmram")))
#endif
	#define NUMBER_OF_STRING 1250
	const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
	{

		"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
		"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of",
		"yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra",
		"equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts",
		"Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me",
		"how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most",
		"interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when", "theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary",
		"Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are", "everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own",
		"Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you", "dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice",
		"to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you", "in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older",
		"you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York", "City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes", "you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will",
		"philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were", "reasonable", "politicians", "were", "noble", "and", "children", "respected", "their", "elders", "Respect", "your", "elders", "Dont", "expect", "anyone", "else", "to", "support", "you", "Maybe", "you", "have", "a", "trust", "fund", "Maybe", "youll", "have", "a", "wealthy", "spouse",
		"But", "you", "never", "know", "when", "either", "one", "might", "run", "out", "Dont", "mess", "too", "much", "with", "your", "hair", "or", "by", "the", "time", "youre", "40", "it", "will", "look", "85", "Be", "careful", "whose", "advice", "you", "buy", "but", "be", "patient", "with", "those", "who", "supply", "it", "Advice", "is", "a", "form", "of", "nostalgia", "Dispensing", "it", "is",
		"a", "way", "of", "fishing", "the", "past", "from", "the", "disposal", "wiping", "it", "off", "painting", "over", "the", "ugly", "parts", "and", "recycling", "it", "for", "more", "than", "its", "worth", "But", "trust", "me", "on", "the", "sunscreen", "Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could",
		"offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis", "more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of",
		"your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of", "yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous",
		"you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra", "equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed",
		"your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts", "Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre",
		"ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me", "how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont",
		"know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most", "interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when",
		"theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary", "Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are",
		"everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own", "Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you",
		"dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice", "to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you",
		"in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older", "you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York",
		"City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes", "you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will", "philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were"
};
#endif


#ifdef STRING_1500
#ifdef INPUT_CCM
	__attribute__((section(".ccmram")))
#endif
	#define NUMBER_OF_STRING 1500
	const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
	{

		"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
		"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of",
		"yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra",
		"equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts",
		"Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me",
		"how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most",
		"interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when", "theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary",
		"Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are", "everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own",
		"Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you", "dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice",
		"to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you", "in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older",
		"you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York", "City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes", "you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will",
		"philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were", "reasonable", "politicians", "were", "noble", "and", "children", "respected", "their", "elders", "Respect", "your", "elders", "Dont", "expect", "anyone", "else", "to", "support", "you", "Maybe", "you", "have", "a", "trust", "fund", "Maybe", "youll", "have", "a", "wealthy", "spouse",
		"But", "you", "never", "know", "when", "either", "one", "might", "run", "out", "Dont", "mess", "too", "much", "with", "your", "hair", "or", "by", "the", "time", "youre", "40", "it", "will", "look", "85", "Be", "careful", "whose", "advice", "you", "buy", "but", "be", "patient", "with", "those", "who", "supply", "it", "Advice", "is", "a", "form", "of", "nostalgia", "Dispensing", "it", "is",
		"a", "way", "of", "fishing", "the", "past", "from", "the", "disposal", "wiping", "it", "off", "painting", "over", "the", "ugly", "parts", "and", "recycling", "it", "for", "more", "than", "its", "worth", "But", "trust", "me", "on", "the", "sunscreen", "Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could",
		"offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis", "more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of",
		"your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of", "yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous",
		"you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra", "equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed",
		"your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts", "Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre",
		"ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me", "how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont",
		"know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most", "interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when",
		"theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary", "Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are",
		"everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own", "Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you",
		"dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice", "to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you",
		"in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older", "you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York",
		"City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes", "you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will", "philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were",
		"reasonable", "politicians", "were", "noble", "and", "children", "respected", "their", "elders", "Respect", "your", "elders", "Dont", "expect", "anyone", "else", "to", "support", "you", "Maybe", "you", "have", "a", "trust", "fund", "Maybe", "youll", "have", "a", "wealthy", "spouse", "But", "you", "never", "know", "when", "either", "one", "might", "run", "out", "Dont", "mess", "too", "much", "with", "your", "hair", "or", "by",
		"the", "time", "youre", "40", "it", "will", "look", "85", "Be", "careful", "whose", "advice", "you", "buy", "but", "be", "patient", "with", "those", "who", "supply", "it", "Advice", "is", "a", "form", "of", "nostalgia", "Dispensing", "it", "is", "a", "way", "of", "fishing", "the", "past", "from", "the", "disposal", "wiping", "it", "off", "painting", "over", "the", "ugly", "parts", "and", "recycling",
		"it", "for", "more", "than", "its", "worth", "But", "trust", "me", "on", "the", "sunscreen", "Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been",
		"proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis", "more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded",
		"But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of", "yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but"
	};
	#endif



//#define NUMBER_OF_STRING 1750
//
//const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
//{
//
//		"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
//		"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of",
//		"yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra",
//		"equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts",
//		"Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me",
//		"how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most",
//		"interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when", "theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary",
//		"Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are", "everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own",
//		"Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you", "dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice",
//		"to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you", "in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older",
//		"you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York", "City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes", "you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will",
//		"philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were", "reasonable", "politicians", "were", "noble", "and", "children", "respected", "their", "elders", "Respect", "your", "elders", "Dont", "expect", "anyone", "else", "to", "support", "you", "Maybe", "you", "have", "a", "trust", "fund", "Maybe", "youll", "have", "a", "wealthy", "spouse",
//		"But", "you", "never", "know", "when", "either", "one", "might", "run", "out", "Dont", "mess", "too", "much", "with", "your", "hair", "or", "by", "the", "time", "youre", "40", "it", "will", "look", "85", "Be", "careful", "whose", "advice", "you", "buy", "but", "be", "patient", "with", "those", "who", "supply", "it", "Advice", "is", "a", "form", "of", "nostalgia", "Dispensing", "it", "is",
//		"a", "way", "of", "fishing", "the", "past", "from", "the", "disposal", "wiping", "it", "off", "painting", "over", "the", "ugly", "parts", "and", "recycling", "it", "for", "more", "than", "its", "worth", "But", "trust", "me", "on", "the", "sunscreen", "Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could",
//		"offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis", "more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of",
//		"your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of", "yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous",
//		"you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra", "equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed",
//		"your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts", "Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre",
//		"ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me", "how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont",
//		"know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most", "interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when",
//		"theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary", "Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are",
//		"everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own", "Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you",
//		"dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice", "to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you",
//		"in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older", "you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York",
//		"City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes", "you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will", "philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were",
//		"reasonable", "politicians", "were", "noble", "and", "children", "respected", "their", "elders", "Respect", "your", "elders", "Dont", "expect", "anyone", "else", "to", "support", "you", "Maybe", "you", "have", "a", "trust", "fund", "Maybe", "youll", "have", "a", "wealthy", "spouse", "But", "you", "never", "know", "when", "either", "one", "might", "run", "out", "Dont", "mess", "too", "much", "with", "your", "hair", "or", "by",
//		"the", "time", "youre", "40", "it", "will", "look", "85", "Be", "careful", "whose", "advice", "you", "buy", "but", "be", "patient", "with", "those", "who", "supply", "it", "Advice", "is", "a", "form", "of", "nostalgia", "Dispensing", "it", "is", "a", "way", "of", "fishing", "the", "past", "from", "the", "disposal", "wiping", "it", "off", "painting", "over", "the", "ugly", "parts", "and", "recycling",
//		"it", "for", "more", "than", "its", "worth", "But", "trust", "me", "on", "the", "sunscreen", "Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been",
//		"proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis", "more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded",
//		"But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of", "yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but",
//		"know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra", "equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every",
//		"day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts", "Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you",
//		"receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me", "how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22",
//		"what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most", "interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when", "theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at",
//		"40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary", "Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are", "everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what"
//};

//
//#define NUMBER_OF_STRING 2000
//
//const char arr[NUMBER_OF_STRING][MAX_STRING_SIZE] =
//{
//
//		"Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis",
//		"more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of",
//		"yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra",
//		"equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts",
//		"Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me",
//		"how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most",
//		"interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when", "theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary",
//		"Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are", "everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own",
//		"Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you", "dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice",
//		"to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you", "in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older",
//		"you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York", "City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes", "you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will",
//		"philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were", "reasonable", "politicians", "were", "noble", "and", "children", "respected", "their", "elders", "Respect", "your", "elders", "Dont", "expect", "anyone", "else", "to", "support", "you", "Maybe", "you", "have", "a", "trust", "fund", "Maybe", "youll", "have", "a", "wealthy", "spouse",
//		"But", "you", "never", "know", "when", "either", "one", "might", "run", "out", "Dont", "mess", "too", "much", "with", "your", "hair", "or", "by", "the", "time", "youre", "40", "it", "will", "look", "85", "Be", "careful", "whose", "advice", "you", "buy", "but", "be", "patient", "with", "those", "who", "supply", "it", "Advice", "is", "a", "form", "of", "nostalgia", "Dispensing", "it", "is",
//		"a", "way", "of", "fishing", "the", "past", "from", "the", "disposal", "wiping", "it", "off", "painting", "over", "the", "ugly", "parts", "and", "recycling", "it", "for", "more", "than", "its", "worth", "But", "trust", "me", "on", "the", "sunscreen", "Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could",
//		"offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been", "proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis", "more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of",
//		"your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded", "But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of", "yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous",
//		"you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but", "know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra", "equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed",
//		"your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every", "day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts", "Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre",
//		"ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you", "receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me", "how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont",
//		"know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22", "what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most", "interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when",
//		"theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at", "40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary", "Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are",
//		"everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what", "other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own", "Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you",
//		"dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your", "parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice", "to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you",
//		"in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work", "hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older", "you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York",
//		"City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes", "you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will", "philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were",
//		"reasonable", "politicians", "were", "noble", "and", "children", "respected", "their", "elders", "Respect", "your", "elders", "Dont", "expect", "anyone", "else", "to", "support", "you", "Maybe", "you", "have", "a", "trust", "fund", "Maybe", "youll", "have", "a", "wealthy", "spouse", "But", "you", "never", "know", "when", "either", "one", "might", "run", "out", "Dont", "mess", "too", "much", "with", "your", "hair", "or", "by",
//		"the", "time", "youre", "40", "it", "will", "look", "85", "Be", "careful", "whose", "advice", "you", "buy", "but", "be", "patient", "with", "those", "who", "supply", "it", "Advice", "is", "a", "form", "of", "nostalgia", "Dispensing", "it", "is", "a", "way", "of", "fishing", "the", "past", "from", "the", "disposal", "wiping", "it", "off", "painting", "over", "the", "ugly", "parts", "and", "recycling",
//		"it", "for", "more", "than", "its", "worth", "But", "trust", "me", "on", "the", "sunscreen", "Kurt", "Vonneguts", "Commencement", "Address", "at", "MIT", "Ladies", "and", "gentlemen", "of", "the", "class", "of", "97", "Wear", "sunscreen", "If", "I", "could", "offer", "you", "only", "one", "tip", "for", "the", "future", "sunscreen", "would", "be", "it", "The", "longterm", "benefits", "of", "sunscreen", "have", "been",
//		"proved", "by", "scientists", "whereas", "the", "rest", "of", "my", "advice", "has", "no", "basis", "more", "reliable", "than", "my", "own", "meandering", "experience", "I", "will", "dispense", "this", "advice", "now", "Enjoy", "the", "power", "and", "beauty", "of", "your", "youth", "Oh", "never", "mind", "You", "will", "not", "understand", "the", "power", "and", "beauty", "of", "your", "youth", "until", "theyve", "faded",
//		"But", "trust", "me", "in", "20", "years", "youll", "look", "back", "at", "photos", "of", "yourself", "and", "recall", "in", "a", "way", "you", "cant", "grasp", "now", "how", "much", "possibility", "lay", "before", "you", "and", "how", "fabulous", "you", "really", "looked", "You", "are", "not", "as", "fat", "as", "you", "imagine", "Dont", "worry", "about", "the", "future", "Or", "worry", "but",
//		"know", "that", "worrying", "is", "as", "effective", "as", "trying", "to", "solve", "an", "algebra", "equation", "by", "chewing", "bubble", "gum", "The", "real", "troubles", "in", "your", "life", "are", "apt", "to", "be", "things", "that", "never", "crossed", "your", "worried", "mind", "the", "kind", "that", "blindside", "you", "at", "4", "pm", "on", "some", "idle", "Tuesday", "Do", "one", "thing", "every",
//		"day", "that", "scares", "you", "Sing", "Dont", "be", "reckless", "with", "other", "peoples", "hearts", "Dont", "put", "up", "with", "people", "who", "are", "reckless", "with", "yours", "Floss", "Dont", "waste", "your", "time", "on", "jealousy", "Sometimes", "youre", "ahead", "sometimes", "youre", "behind", "The", "race", "is", "long", "and", "in", "the", "end", "its", "only", "with", "yourself", "Remember", "compliments", "you",
//		"receive", "Forget", "the", "insults", "If", "you", "succeed", "in", "doing", "this", "tell", "me", "how", "Keep", "your", "old", "love", "letters", "Throw", "away", "your", "old", "bank", "statements", "Stretch", "Dont", "feel", "guilty", "if", "you", "dont", "know", "what", "you", "want", "to", "do", "with", "your", "life", "The", "most", "interesting", "people", "I", "know", "didnt", "know", "at", "22",
//		"what", "they", "wanted", "to", "do", "with", "their", "lives", "Some", "of", "the", "most", "interesting", "40yearolds", "I", "know", "still", "dont", "Get", "plenty", "of", "calcium", "Be", "kind", "to", "your", "knees", "Youll", "miss", "them", "when", "theyre", "gone", "Maybe", "youll", "marry", "maybe", "you", "wont", "Maybe", "youll", "have", "children", "maybe", "you", "wont", "Maybe", "youll", "divorce", "at",
//		"40", "maybe", "youll", "dance", "the", "funky", "chicken", "on", "your", "75th", "wedding", "anniversary", "Whatever", "you", "do", "dont", "congratulate", "yourself", "too", "much", "or", "berate", "yourself", "either", "Your", "choices", "are", "half", "chance", "So", "are", "everybody", "elses", "Enjoy", "your", "body", "Use", "it", "every", "way", "you", "can", "Dont", "be", "afraid", "of", "it", "or", "of", "what",
//		"other", "people", "think", "of", "it", "Its", "the", "greatest", "instrument", "youll", "ever", "own", "Dance", "even", "if", "you", "have", "nowhere", "to", "do", "it", "but", "your", "living", "room", "Read", "the", "directions", "even", "if", "you", "dont", "follow", "them", "Do", "not", "read", "beauty", "magazines", "They", "will", "only", "make", "you", "feel", "ugly", "Get", "to", "know", "your",
//		"parents", "You", "never", "know", "when", "theyll", "be", "gone", "for", "good", "Be", "nice", "to", "your", "siblings", "Theyre", "your", "best", "link", "to", "your", "past", "and", "the", "people", "most", "likely", "to", "stick", "with", "you", "in", "the", "future", "Understand", "that", "friends", "come", "and", "go", "but", "with", "a", "precious", "few", "you", "should", "hold", "on", "Work",
//		"hard", "to", "bridge", "the", "gaps", "in", "geography", "and", "lifestyle", "because", "the", "older", "you", "get", "the", "more", "you", "need", "the", "people", "who", "knew", "you", "when", "you", "were", "young", "Live", "in", "New", "York", "City", "once", "but", "leave", "before", "it", "makes", "you", "hard", "Live", "in", "Northern", "California", "once", "but", "leave", "before", "it", "makes",
//		"you", "soft", "Travel", "Accept", "certain", "inalienable", "truths", "Prices", "will", "rise", "Politicians", "will", "philander", "You", "too", "will", "get", "old", "And", "when", "you", "do", "youll", "fantasize", "that", "when", "you", "were", "young", "prices", "were", "reasonable", "politicians", "were", "noble", "and", "children", "respected", "their", "elders", "Respect", "your", "elders", "Dont", "expect", "anyone", "else", "to", "support", "you",
//		"Maybe", "you", "have", "a", "trust", "fund", "Maybe", "youll", "have", "a", "wealthy", "spouse", "But", "you", "never", "know", "when", "either", "one", "might", "run", "out", "Dont", "mess", "too", "much", "with", "your", "hair", "or", "by", "the", "time", "youre", "40", "it", "will", "look", "85", "Be", "careful", "whose", "advice", "you", "buy", "but", "be", "patient", "with", "those"
//
//};
//



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  int time1 __attribute__((aligned (4))) = 0;
  int time2 __attribute__((aligned (4))) = 0;

  //void volatile **this_pp __attribute__((aligned (4))) = p0;
  //void volatile **this_pp __attribute__((aligned (4))) = &pointers[0];

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  struct myStringStruct array[NUMBER_OF_STRING];
  // enable update interrupts
  TIM2->DIER|=TIM_DIER_UIE;

////////////////////////////////////////////////////////////////////start counting time
  HAL_TIM_Base_Start(&htim2);
  overflow_cnt = 0;
  for(int i=0;i<NUMBER_OF_STRING;i++)
        strcpy(array[i].qstring, arr[i]);


  time1 = TIM2->CNT;



  my_qsort(array,NUMBER_OF_STRING,sizeof(struct myStringStruct),compare);

  time2 = TIM2->CNT;
  ////////////////////////////////////////////////////////////////////end counting time

  tim_cnt = time2 -time1;
  execution_time = overflow_cnt*1000 + (double)tim_cnt/(1000);
  uint8_t msg[40] = {'\0'};

  sprintf(msg,"\n\r%d", (int)(execution_time*1000));
  HAL_UART_Transmit(&huart1, msg, sizeof(msg), 0xffff);
  //execution time is in ms

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  return 0;
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = (24-1);
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = (1000000-1);
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_EVEN;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	overflow_cnt++;

}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
