/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <types.hpp>

namespace Impl
{

inline void getRandomVehicleColour(int modelid, int& colour1, int& colour2, int& colour3, int& colour4)
{
	static const uint16_t vehicleIndexes[212 + 1] = {
		0, // 400 - landstal
		8, // 401 - bravura
		16, // 402 - buffalo
		24, // 403 - linerun
		32, // 404 - peren
		40, // 405 - sentinel
		48, // 406 - dumper
		49, // 407 - firetruk
		50, // 408 - trash
		51, // 409 - stretch
		52, // 410 - manana
		60, // 411 - infernus
		68, // 412 - voodoo
		76, // 413 - pony
		84, // 414 - mule
		92, // 415 - cheetah
		100, // 416 - ambulan
		101, // 417 - leviathn
		101, // 418 - moonbeam
		109, // 419 - esperant
		117, // 420 - taxi
		118, // 421 - washing
		126, // 422 - bobcat
		133, // 423 - mrwhoop
		139, // 424 - bfinject
		147, // 425 - hunter
		148, // 426 - premier
		156, // 427 - enforcer
		157, // 428 - securica
		158, // 429 - banshee
		166, // 430 - predator
		167, // 431 - bus
		175, // 432 - rhino
		176, // 433 - barracks
		177, // 434 - hotknife
		184, // 435 - artict1
		185, // 436 - previon
		192, // 437 - coach
		200, // 438 - cabbie
		201, // 439 - stallion
		209, // 440 - rumpo
		217, // 441 - rcbandit
		225, // 442 - romero
		233, // 443 - packer
		241, // 444 - monster
		247, // 445 - admiral
		255, // 446 - squalo
		263, // 447 - seaspar
		264, // 448 - pizzaboy
		265, // 449 - tram
		266, // 450 - artict2
		267, // 451 - turismo
		275, // 452 - speeder
		283, // 453 - reefer
		284, // 454 - tropic
		285, // 455 - flatbed
		293, // 456 - yankee
		301, // 457 - caddy
		309, // 458 - solair
		317, // 459 - topfun
		325, // 460 - skimmer
		333, // 461 - pcj600
		341, // 462 - faggio
		349, // 463 - freeway
		357, // 464 - rcbaron
		358, // 465 - rcraider
		359, // 466 - glendale
		367, // 467 - oceanic
		375, // 468 - sanchez
		379, // 469 - sparrow
		380, // 470 - patriot
		381, // 471 - quad
		389, // 472 - coastg
		391, // 473 - dinghy
		393, // 474 - hermes
		401, // 475 - sabre
		409, // 476 - rustler
		417, // 477 - zr350
		425, // 478 - walton
		433, // 479 - regina
		441, // 480 - comet
		448, // 481 - bmx
		456, // 482 - burrito
		464, // 483 - camper
		472, // 484 - marquis
		476, // 485 - baggage
		483, // 486 - dozer
		484, // 487 - maverick
		492, // 488 - vcnmav
		494, // 489 - rancher
		500, // 490 - fbiranch
		501, // 491 - virgo
		507, // 492 - greenwoo
		515, // 493 - jetmax
		516, // 494 - hotring
		524, // 495 - sandking
		532, // 496 - blistac
		540, // 497 - polmav
		541, // 498 - boxville
		549, // 499 - benson
		557, // 500 - mesa
		565, // 501 - rcgoblin
		566, // 502 - hotrina
		574, // 503 - hotrinb
		582, // 504 - bloodra
		590, // 505 - rnchlure
		596, // 506 - supergt
		601, // 507 - elegant
		609, // 508 - journey
		610, // 509 - bike
		618, // 510 - mtbike
		626, // 511 - beagle
		634, // 512 - cropdust
		642, // 513 - stunt
		650, // 514 - petro
		658, // 515 - rdtrain
		666, // 516 - nebula
		674, // 517 - majestic
		682, // 518 - buccanee
		690, // 519 - shamal
		691, // 520 - hydra
		691, // 521 - fcr900
		699, // 522 - nrg500
		707, // 523 - copbike
		707, // 524 - cement
		715, // 525 - towtruck
		722, // 526 - fortune
		730, // 527 - cadrona
		738, // 528 - fbitruck
		738, // 529 - willard
		746, // 530 - forklift
		754, // 531 - tractor
		760, // 532 - combine
		760, // 533 - feltzer
		768, // 534 - remingtn
		776, // 535 - slamvan
		784, // 536 - blade
		792, // 537 - freight
		793, // 538 - streak
		794, // 539 - vortex
		801, // 540 - vincent
		809, // 541 - bullet
		817, // 542 - clover
		825, // 543 - sadler
		833, // 544 - firela
		834, // 545 - hustler
		842, // 546 - intruder
		850, // 547 - primo
		858, // 548 - cargobob
		859, // 549 - tampa
		867, // 550 - sunrise
		875, // 551 - merit
		883, // 552 - utility
		886, // 553 - nevada
		894, // 554 - yosemite
		902, // 555 - windsor
		910, // 556 - monstera
		911, // 557 - monsterb
		912, // 558 - uranus
		920, // 559 - jester
		928, // 560 - sultan
		936, // 561 - stratum
		944, // 562 - elegy
		952, // 563 - raindanc
		953, // 564 - rctiger
		953, // 565 - flash
		961, // 566 - tahoma
		969, // 567 - savanna
		977, // 568 - bandito
		985, // 569 - freiflat
		985, // 570 - streakc
		986, // 571 - kart
		992, // 572 - mower
		1000, // 573 - duneride
		1008, // 574 - sweeper
		1009, // 575 - broadway
		1017, // 576 - tornado
		1025, // 577 - at400
		1031, // 578 - dft30
		1032, // 579 - huntley
		1040, // 580 - stafford
		1049, // 581 - bf400
		1057, // 582 - newsvan
		1065, // 583 - tug
		1066, // 584 - petrotr
		1067, // 585 - emperor
		1075, // 586 - wayfarer
		1083, // 587 - euros
		1091, // 588 - hotdog
		1092, // 589 - club
		1100, // 590 - freibox
		1100, // 591 - artict3
		1101, // 592 - androm
		1102, // 593 - dodo
		1110, // 594 - rccam
		1110, // 595 - launch
		1111, // 596 - copcarla
		1112, // 597 - copcarsf
		1113, // 598 - copcarvg
		1114, // 599 - copcarru
		1115, // 600 - picador
		1123, // 601 - swatvan
		1124, // 602 - alpha
		1132, // 603 - phoenix
		1140, // 604 - glenshit
		1148, // 605 - sadlshit
		1156, // 606 - bagboxa
		1156, // 607 - bagboxb
		1156, // 608 - tugstair
		1157, // 609 - boxburg
		1158, // 610 - farmtr1
		1158, // 611 - utiltr1
		1158
	};

	static const uint8_t vehiclePrimaryColours[1158] = {
		4, 123, 113, 101, 75, 62, 40, 36, // 400 - landstal
		41, 47, 52, 66, 74, 87, 91, 113, // 401 - bravura
		10, 13, 22, 30, 39, 90, 98, 110, // 402 - buffalo
		36, 37, 30, 28, 25, 40, 101, 113, // 403 - linerun
		113, 119, 123, 109, 101, 95, 83, 66, // 404 - peren
		11, 24, 36, 40, 75, 91, 123, 4, // 405 - sentinel
		1, // 406 - dumper
		3, // 407 - firetruk
		26, // 408 - trash
		1, // 409 - stretch
		4, 9, 10, 25, 36, 40, 45, 84, // 410 - manana
		12, 64, 123, 116, 112, 106, 80, 75, // 411 - infernus
		9, 10, 11, 25, 27, 29, 30, 37, // 412 - voodoo
		87, 88, 91, 105, 109, 119, 4, 25, // 413 - pony
		25, 28, 43, 67, 72, 9, 95, 24, // 414 - mule
		20, 25, 36, 40, 62, 75, 92, 0, // 415 - cheetah
		1, // 416 - ambulan
		// 417 - leviathn
		119, 117, 114, 108, 95, 81, 61, 41, // 418 - moonbeam
		45, 47, 33, 13, 54, 69, 59, 87, // 419 - esperant
		6, // 420 - taxi
		4, 13, 25, 30, 36, 40, 75, 95, // 421 - washing
		96, 97, 101, 111, 113, 83, 67, // 422 - bobcat
		1, 1, 1, 1, 1, 1, // 423 - mrwhoop
		1, 2, 3, 3, 6, 15, 24, 35, // 424 - bfinject
		43, // 425 - hunter
		37, 42, 53, 62, 7, 10, 11, 15, // 426 - premier
		0, // 427 - enforcer
		4, // 428 - securica
		12, 13, 14, 1, 2, 1, 3, 10, // 429 - banshee
		46, // 430 - predator
		71, 75, 92, 47, 55, 59, 71, 82, // 431 - bus
		43, // 432 - rhino
		43, // 433 - barracks
		1, 12, 2, 6, 4, 46, 53, // 434 - hotknife
		1, // 435 - artict1
		83, 87, 92, 95, 109, 119, 11, // 436 - previon
		54, 79, 87, 95, 98, 105, 123, 125, // 437 - coach
		6, // 438 - cabbie
		57, 8, 43, 54, 67, 37, 65, 25, // 439 - stallion
		34, 32, 20, 110, 66, 84, 118, 121, // 440 - rumpo
		2, 79, 82, 67, 126, 70, 110, 67, // 441 - rcbandit
		0, 11, 25, 36, 40, 75, 0, 0, // 442 - romero
		4, 20, 24, 25, 36, 40, 54, 84, // 443 - packer
		32, 32, 32, 32, 32, 32, // 444 - monster
		34, 35, 37, 39, 41, 43, 45, 47, // 445 - admiral
		0, 1, 3, 1, 1, 1, 1, 1, // 446 - squalo
		75, // 447 - seaspar
		3, // 448 - pizzaboy
		1, // 449 - tram
		1, // 450 - artict2
		123, 125, 36, 16, 18, 46, 61, 75, // 451 - turismo
		1, 1, 1, 1, 1, 1, 1, 1, // 452 - speeder
		56, // 453 - reefer
		26, // 454 - tropic
		84, 84, 84, 32, 43, 1, 77, 32, // 455 - flatbed
		84, 91, 102, 105, 110, 121, 12, 23, // 456 - yankee
		58, 2, 63, 18, 32, 45, 13, 34, // 457 - caddy
		91, 101, 109, 113, 4, 25, 30, 36, // 458 - solair
		26, 28, 44, 51, 57, 72, 106, 112, // 459 - topfun
		1, 1, 1, 1, 17, 46, 46, 57, // 460 - skimmer
		36, 37, 43, 53, 61, 75, 79, 88, // 461 - pcj600
		12, 13, 14, 1, 2, 1, 3, 10, // 462 - faggio
		79, 84, 7, 11, 19, 22, 36, 53, // 463 - freeway
		14, // 464 - rcbaron
		14, // 465 - rcraider
		67, 68, 78, 2, 16, 18, 25, 45, // 466 - glendale
		51, 58, 60, 68, 2, 13, 22, 36, // 467 - oceanic
		6, 46, 53, 3, // 468 - sanchez
		1, // 469 - sparrow
		43, // 470 - patriot
		120, 103, 120, 74, 120, 74, 120, 66, // 471 - quad
		56, 56, // 472 - coastg
		56, 56, // 473 - dinghy
		97, 81, 105, 110, 91, 74, 84, 83, // 474 - hermes
		2, 9, 17, 21, 33, 37, 41, 56, // 475 - sabre
		6, 7, 1, 89, 119, 103, 77, 71, // 476 - rustler
		92, 94, 101, 121, 0, 22, 36, 75, // 477 - zr350
		72, 66, 59, 45, 40, 39, 35, 20, // 478 - walton
		27, 59, 60, 55, 54, 49, 45, 40, // 479 - regina
		73, 12, 2, 6, 4, 46, 53, // 480 - comet
		1, 3, 6, 46, 65, 14, 12, 26, // 481 - bmx
		41, 48, 52, 64, 71, 85, 10, 62, // 482 - burrito
		1, 1, 1, 1, 0, 3, 16, 17, // 483 - camper
		12, 50, 40, 66, // 484 - marquis
		1, 1, 1, 1, 1, 1, 1, // 485 - baggage
		1, // 486 - dozer
		26, 29, 26, 54, 26, 3, 12, 74, // 487 - maverick
		2, 2, // 488 - vcnmav
		13, 14, 120, 112, 84, 76, // 489 - rancher
		0, // 490 - fbiranch
		40, 71, 52, 64, 30, 60, // 491 - virgo
		30, 77, 81, 24, 28, 49, 52, 71, // 492 - greenwoo
		36, // 493 - jetmax
		36, 36, 42, 42, 54, 75, 92, 98, // 494 - hotring
		123, 119, 118, 116, 114, 101, 88, 5, // 495 - sandking
		74, 66, 53, 37, 22, 20, 9, 0, // 496 - blistac
		0, // 497 - polmav
		11, 13, 20, 24, 27, 36, 37, 43, // 498 - boxville
		109, 109, 112, 10, 30, 32, 84, 84, // 499 - benson
		75, 40, 40, 28, 25, 21, 13, 4, // 500 - mesa
		14, // 501 - rcgoblin
		7, 36, 51, 53, 58, 75, 75, 79, // 502 - hotrina
		83, 87, 87, 98, 101, 103, 117, 123, // 503 - hotrinb
		51, 57, 45, 34, 65, 14, 12, 26, // 504 - bloodra
		13, 14, 120, 112, 84, 76, // 505 - rnchlure
		3, 6, 7, 52, 76, // 506 - supergt
		37, 42, 53, 62, 7, 10, 11, 15, // 507 - elegant
		1, // 508 - journey
		7, 74, 61, 16, 25, 30, 36, 53, // 509 - bike
		43, 46, 39, 28, 16, 6, 5, 2, // 510 - mtbike
		3, 4, 7, 8, 12, 27, 34, 37, // 511 - beagle
		17, 15, 32, 45, 52, 57, 61, 96, // 512 - cropdust
		38, 21, 21, 30, 54, 55, 48, 51, // 513 - stunt
		10, 25, 28, 36, 40, 54, 75, 113, // 514 - petro
		13, 24, 63, 42, 54, 39, 11, 62, // 515 - rdtrain
		116, 119, 122, 4, 9, 24, 27, 36, // 516 - nebula
		37, 36, 40, 43, 47, 51, 54, 55, // 517 - majestic
		2, 9, 17, 21, 33, 37, 41, 56, // 518 - buccanee
		1, // 519 - shamal
		// 520 - hydra
		74, 75, 87, 92, 115, 25, 36, 118, // 521 - fcr900
		3, 3, 6, 7, 8, 36, 39, 51, // 522 - nrg500
		// 523 - copbike
		60, 61, 65, 61, 81, 62, 83, 83, // 524 - cement
		1, 17, 18, 22, 36, 44, 52, // 525 - towtruck
		2, 9, 17, 21, 33, 37, 41, 56, // 526 - fortune
		52, 53, 66, 75, 76, 81, 95, 109, // 527 - cadrona
		// 528 - fbitruck
		37, 42, 53, 62, 7, 10, 11, 15, // 529 - willard
		110, 111, 112, 114, 119, 122, 4, 13, // 530 - forklift
		2, 36, 51, 91, 11, 40, // 531 - tractor
		// 532 - combine
		73, 74, 75, 77, 79, 83, 84, 91, // 533 - feltzer
		37, 42, 53, 62, 7, 10, 11, 15, // 534 - remingtn
		3, 28, 31, 55, 66, 97, 123, 118, // 535 - slamvan
		9, 12, 26, 30, 32, 37, 57, 71, // 536 - blade
		1, // 537 - freight
		1, // 538 - streak
		96, 86, 79, 70, 61, 75, 75, // 539 - vortex
		37, 42, 53, 62, 7, 10, 11, 15, // 540 - vincent
		51, 58, 60, 68, 2, 13, 22, 36, // 541 - bullet
		13, 24, 31, 32, 45, 113, 119, 122, // 542 - clover
		76, 32, 43, 67, 11, 8, 2, 83, // 543 - sadler
		3, // 544 - firela
		50, 47, 44, 40, 39, 30, 28, 9, // 545 - hustler
		62, 78, 2, 3, 2, 113, 119, 7, // 546 - intruder
		122, 123, 125, 10, 24, 37, 55, 66, // 547 - primo
		1, // 548 - cargobob
		74, 72, 75, 79, 83, 84, 89, 91, // 549 - tampa
		37, 42, 53, 62, 7, 10, 11, 15, // 550 - sunrise
		67, 72, 75, 83, 91, 101, 109, 20, // 551 - merit
		56, 49, 26, // 552 - utility
		38, 55, 61, 71, 91, 98, 102, 111, // 553 - nevada
		53, 15, 45, 34, 65, 14, 12, 43, // 554 - yosemite
		51, 58, 60, 68, 2, 13, 22, 36, // 555 - windsor
		1, // 556 - monstera
		1, // 557 - monsterb
		112, 116, 117, 24, 30, 35, 36, 40, // 558 - uranus
		51, 58, 60, 68, 2, 13, 22, 36, // 559 - jester
		52, 9, 17, 21, 33, 37, 41, 56, // 560 - sultan
		57, 8, 43, 54, 67, 37, 65, 25, // 561 - stratum
		36, 35, 17, 11, 116, 113, 101, 92, // 562 - elegy
		1, // 563 - raindanc
		// 564 - rctiger
		37, 42, 53, 62, 7, 10, 11, 15, // 565 - flash
		109, 30, 95, 84, 83, 72, 71, 52, // 566 - tahoma
		97, 88, 90, 93, 97, 99, 102, 114, // 567 - savanna
		2, 9, 17, 21, 33, 37, 41, 56, // 568 - bandito
		// 569 - freiflat
		1, // 570 - streakc
		2, 36, 51, 91, 11, 40, // 571 - kart
		94, 101, 116, 117, 4, 25, 30, 37, // 572 - mower
		91, 115, 85, 79, 78, 77, 79, 86, // 573 - duneride
		26, // 574 - sweeper
		12, 19, 31, 25, 38, 51, 57, 66, // 575 - broadway
		67, 68, 72, 74, 75, 76, 79, 84, // 576 - tornado
		1, 8, 8, 8, 23, 40, // 577 - at400
		1, // 578 - dft30
		37, 42, 53, 62, 7, 10, 11, 15, // 579 - huntley
		92, 81, 67, 66, 61, 53, 51, 47, 43, // 580 - stafford
		54, 58, 66, 72, 75, 87, 101, 36, // 581 - bf400
		41, 41, 49, 56, 110, 112, 114, 119, // 582 - newsvan
		1, // 583 - tug
		1, // 584 - petrotr
		37, 42, 53, 62, 7, 10, 11, 15, // 585 - emperor
		119, 122, 8, 10, 13, 25, 27, 32, // 586 - wayfarer
		36, 40, 43, 53, 72, 75, 95, 101, // 587 - euros
		1, // 588 - hotdog
		37, 31, 23, 22, 7, 124, 114, 112, // 589 - club
		// 590 - freibox
		1, // 591 - artict3
		1, // 592 - androm
		51, 58, 60, 68, 2, 13, 22, 36, // 593 - dodo
		// 594 - rccam
		112, // 595 - launch
		0, // 596 - copcarla
		0, // 597 - copcarsf
		0, // 598 - copcarvg
		0, // 599 - copcarru
		81, 32, 43, 67, 11, 8, 2, 83, // 600 - picador
		1, // 601 - swatvan
		58, 69, 75, 18, 32, 45, 13, 34, // 602 - alpha
		58, 69, 75, 18, 32, 45, 13, 34, // 603 - phoenix
		67, 68, 78, 2, 16, 18, 25, 45, // 604 - glenshit
		61, 32, 43, 67, 11, 8, 2, 83, // 605 - sadlshit
		// 606 - bagboxa
		// 607 - bagboxb
		1, // 608 - tugstair
		36 // 609 - boxburg
		// 610 - farmtr1
		// 611 - utiltr1
	};

	static const uint16_t vehicleSecondaryColours[1158] = {
		1, 1, 1, 1, 1, 1, 1, 1, // 400 - landstal
		41, 47, 52, 66, 74, 87, 91, 113, // 401 - bravura
		10, 13, 22, 30, 39, 90, 98, 110, // 402 - buffalo
		1, 1, 1, 1, 1, 1, 1, 1, // 403 - linerun
		39, 50, 92, 100, 101, 105, 110, 25, // 404 - peren
		1, 1, 1, 1, 1, 1, 1, 1, // 405 - sentinel
		1, // 406 - dumper
		1, // 407 - firetruk
		26, // 408 - trash
		1, // 409 - stretch
		1, 1, 1, 1, 1, 1, 1, 1, // 410 - manana
		1, 1, 1, 1, 1, 1, 1, 1, // 411 - infernus
		1, 8, 1, 8, 1, 8, 1, 8, // 412 - voodoo
		1, 1, 1, 1, 1, 1, 1, 1, // 413 - pony
		1, 1, 1, 1, 1, 1, 1, 1, // 414 - mule
		1, 1, 1, 1, 1, 1, 1, 1, // 415 - cheetah
		3, // 416 - ambulan
		// 417 - leviathn
		119, 227, 114, 108, 95, 81, 61, 41, // 418 - moonbeam
		75, 76, 75, 76, 75, 76, 75, 76, // 419 - esperant
		1, // 420 - taxi
		1, 1, 1, 1, 1, 1, 1, 1, // 421 - washing
		25, 25, 25, 31, 36, 57, 59, // 422 - bobcat
		16, 56, 17, 53, 5, 35, // 423 - mrwhoop
		0, 2, 2, 6, 16, 30, 53, 61, // 424 - bfinject
		0, // 425 - hunter
		37, 42, 53, 62, 7, 10, 11, 15, // 426 - premier
		1, // 427 - enforcer
		75, // 428 - securica
		12, 13, 14, 2, 1, 3, 1, 10, // 429 - banshee
		26, // 430 - predator
		59, 59, 72, 74, 83, 83, 87, 87, // 431 - bus
		0, // 432 - rhino
		0, // 433 - barracks
		1, 12, 2, 6, 4, 46, 53, // 434 - hotknife
		1, // 435 - artict1
		1, 1, 1, 1, 1, 45, 1, // 436 - previon
		7, 7, 7, 16, 20, 20, 20, 21, // 437 - coach
		76, // 438 - cabbie
		8, 17, 21, 38, 8, 78, 79, 78, // 439 - stallion
		34, 32, 20, 110, 66, 84, 118, 121, // 440 - rumpo
		96, 42, 54, 86, 96, 96, 54, 98, // 441 - rcbandit
		0, 105, 109, 0, 36, 36, 36, 109, // 442 - romero
		1, 1, 1, 1, 1, 1, 1, 1, // 443 - packer
		36, 42, 53, 66, 14, 32, // 444 - monster
		34, 35, 37, 39, 41, 43, 45, 47, // 445 - admiral
		0, 5, 3, 22, 35, 44, 53, 57, // 446 - squalo
		2, // 447 - seaspar
		6, // 448 - pizzaboy
		74, // 449 - tram
		1, // 450 - artict2
		123, 125, 36, 16, 18, 46, 61, 75, // 451 - turismo
		3, 5, 16, 22, 35, 44, 53, 57, // 452 - speeder
		56, // 453 - reefer
		26, // 454 - tropic
		15, 58, 31, 74, 31, 31, 31, 74, // 455 - flatbed
		63, 63, 65, 72, 93, 93, 95, 1, // 456 - yankee
		1, 1, 1, 1, 1, 1, 1, 1, // 457 - caddy
		1, 1, 1, 1, 1, 1, 1, 1, // 458 - solair
		26, 28, 44, 51, 57, 72, 106, 112, // 459 - topfun
		3, 9, 18, 30, 23, 23, 32, 34, // 460 - skimmer
		1, 1, 1, 1, 1, 1, 1, 1, // 461 - pcj600
		12, 13, 14, 2, 1, 3, 1, 10, // 462 - faggio
		79, 84, 7, 11, 19, 22, 36, 53, // 463 - freeway
		75, // 464 - rcbaron
		75, // 465 - rcraider
		76, 76, 76, 76, 76, 76, 76, 88, // 466 - glendale
		1, 8, 1, 8, 1, 8, 1, 8, // 467 - oceanic
		6, 46, 53, 3, // 468 - sanchez
		3, // 469 - sparrow
		0, // 470 - patriot
		117, 111, 114, 91, 112, 83, 113, 71, // 471 - quad
		15, 53, // 472 - coastg
		15, 53, // 473 - dinghy
		1, 1, 1, 1, 1, 1, 1, 1, // 474 - hermes
		39, 39, 1, 1, 0, 0, 29, 29, // 475 - sabre
		7, 6, 6, 91, 117, 102, 87, 77, // 476 - rustler
		1, 1, 1, 1, 1, 1, 1, 1, // 477 - zr350
		1, 1, 1, 1, 1, 1, 1, 1, // 478 - walton
		36, 36, 35, 41, 31, 23, 32, 29, // 479 - regina
		45, 12, 2, 6, 4, 46, 53, // 480 - comet
		1, 3, 6, 46, 9, 1, 9, 1, // 481 - bmx
		41, 48, 52, 64, 71, 85, 10, 62, // 482 - burrito
		31, 31, 20, 5, 6, 6, 0, 0, // 483 - camper
		35, 32, 26, 36, // 484 - marquis
		73, 74, 75, 76, 77, 78, 79, // 485 - baggage
		1, // 486 - dozer
		14, 42, 57, 29, 3, 29, 39, 35, // 487 - maverick
		26, 29, // 488 - vcnmav
		118, 123, 123, 120, 110, 102, // 489 - rancher
		0, // 490 - fbiranch
		65, 72, 66, 72, 72, 72, // 491 - virgo
		26, 26, 27, 55, 56, 59, 69, 107, // 492 - greenwoo
		13, // 493 - jetmax
		117, 13, 30, 33, 36, 79, 101, 109, // 494 - hotring
		124, 122, 117, 115, 108, 106, 99, 6, // 495 - sandking
		72, 72, 56, 19, 22, 20, 14, 0, // 496 - blistac
		1, // 497 - polmav
		123, 120, 117, 112, 107, 105, 107, 93, // 498 - boxville
		25, 32, 32, 32, 44, 52, 66, 69, // 499 - benson
		84, 84, 110, 119, 119, 119, 119, 119, // 500 - mesa
		75, // 501 - rcgoblin
		94, 88, 75, 75, 67, 67, 61, 62, // 502 - hotrina
		66, 74, 75, 83, 100, 101, 116, 36, // 503 - hotrinb
		39, 38, 29, 9, 9, 1, 9, 1, // 504 - bloodra
		118, 123, 123, 120, 110, 102, // 505 - rnchlure
		3, 6, 7, 52, 76, // 506 - supergt
		37, 42, 53, 62, 7, 10, 11, 15, // 507 - elegant
		1, // 508 - journey
		1, 1, 1, 1, 1, 1, 1, 1, // 509 - bike
		43, 46, 39, 28, 16, 6, 5, 2, // 510 - mtbike
		90, 90, 68, 66, 60, 97, 51, 51, // 511 - beagle
		39, 123, 112, 88, 71, 67, 96, 96, // 512 - cropdust
		51, 36, 34, 34, 34, 20, 18, 6, // 513 - stunt
		1, 1, 1, 1, 1, 1, 1, 1, // 514 - petro
		76, 77, 78, 76, 77, 78, 76, 77, // 515 - rdtrain
		1, 1, 1, 1, 1, 1, 1, 1, // 516 - nebula
		36, 36, 36, 41, 41, 72, 75, 84, // 517 - majestic
		39, 39, 1, 1, 0, 0, 29, 29, // 518 - buccanee
		1, // 519 - shamal
		// 520 - hydra
		74, 13, 118, 3, 118, 118, 0, 118, // 521 - fcr900
		3, 8, 25, 79, 82, 105, 106, 118, // 522 - nrg500
		// 523 - copbike
		24, 27, 31, 61, 35, 61, 66, 64, // 524 - cement
		1, 20, 20, 30, 43, 51, 54, // 525 - towtruck
		39, 39, 1, 1, 0, 0, 29, 29, // 526 - fortune
		1, 1, 1, 1, 1, 1, 1, 1, // 527 - cadrona
		// 528 - fbitruck
		37, 42, 53, 62, 7, 10, 11, 15, // 529 - willard
		1, 1, 1, 1, 1, 1, 1, 1, // 530 - forklift
		35, 2, 53, 2, 22, 35, // 531 - tractor
		// 532 - combine
		1, 1, 1, 1, 1, 1, 1, 1, // 533 - feltzer
		37, 42, 53, 62, 7, 10, 11, 15, // 534 - remingtn
		1, 1, 1, 1, 1, 1, 1, 1, // 535 - slamvan
		1, 1, 96, 96, 1, 1, 96, 96, // 536 - blade
		1, // 537 - freight
		1, // 538 - streak
		67, 70, 74, 86, 98, 75, 91, // 539 - vortex
		37, 42, 53, 62, 7, 10, 11, 15, // 540 - vincent
		1, 8, 1, 8, 1, 8, 1, 8, // 541 - bullet
		118, 118, 93, 92, 92, 92, 113, 113, // 542 - clover
		8, 8, 8, 8, 11, 90, 2, 13, // 543 - sadler
		1, // 544 - firela
		1, 1, 96, 96, 1, 1, 96, 96, // 545 - hustler
		37, 38, 62, 87, 78, 78, 62, 78, // 546 - intruder
		1, 1, 1, 1, 1, 1, 1, 1, // 547 - primo
		1, // 548 - cargobob
		39, 39, 39, 39, 36, 36, 35, 35, // 549 - tampa
		37, 42, 53, 62, 7, 10, 11, 15, // 550 - sunrise
		1, 1, 1, 1, 1, 1, 1, 1, // 551 - merit
		56, 49, 124, // 552 - utility
		9, 23, 74, 87, 87, 114, 119, 3, // 553 - nevada
		32, 32, 32, 30, 32, 32, 32, 32, // 554 - yosemite
		1, 1, 1, 1, 1, 1, 1, 1, // 555 - windsor
		1, // 556 - monstera
		1, // 557 - monsterb
		1, 1, 1, 1, 1, 1, 1, 1, // 558 - uranus
		1, 8, 1, 8, 1, 8, 1, 8, // 559 - jester
		39, 39, 1, 1, 0, 0, 29, 29, // 560 - sultan
		8, 17, 21, 38, 8, 78, 79, 78, // 561 - stratum
		1, 1, 1, 1, 1, 1, 1, 1, // 562 - elegy
		6, // 563 - raindanc
		// 564 - rctiger
		37, 42, 53, 62, 7, 10, 11, 15, // 565 - flash
		1, 8, 1, 8, 1, 8, 1, 8, // 566 - tahoma
		96, 64, 96, 64, 96, 81, 114, 1, // 567 - savanna
		39, 39, 1, 1, 0, 0, 29, 29, // 568 - bandito
		// 569 - freiflat
		1, // 570 - streakc
		35, 2, 53, 2, 22, 35, // 571 - kart
		1, 1, 1, 1, 1, 1, 1, 1, // 572 - mower
		38, 43, 6, 7, 8, 18, 18, 24, // 573 - duneride
		26, // 574 - sweeper
		1, 96, 64, 96, 1, 96, 1, 96, // 575 - broadway
		1, 96, 1, 8, 96, 8, 1, 96, // 576 - tornado
		3, 7, 10, 16, 31, 44, // 577 - at400
		1, // 578 - dft30
		37, 42, 53, 62, 7, 10, 11, 15, // 579 - huntley
		92, 81, 67, 66, 61, 53, 51, 47, 43, // 580 - stafford
		1, 1, 1, 1, 1, 1, 1, 1, // 581 - bf400
		10, 20, 11, 123, 113, 116, 118, 101, // 582 - newsvan
		1, // 583 - tug
		1, // 584 - petrotr
		37, 42, 53, 62, 7, 10, 11, 15, // 585 - emperor
		1, 1, 1, 1, 1, 1, 1, 1, // 586 - wayfarer
		1, 1, 1, 1, 1, 1, 1, 1, // 587 - euros
		1, // 588 - hotdog
		37, 31, 23, 22, 7, 124, 114, 112, // 589 - club
		// 590 - freibox
		1, // 591 - artict3
		1, // 592 - androm
		1, 8, 1, 8, 1, 8, 1, 8, // 593 - dodo
		// 594 - rccam
		20, // 595 - launch
		1, // 596 - copcarla
		1, // 597 - copcarsf
		1, // 598 - copcarvg
		1, // 599 - copcarru
		8, 8, 8, 8, 11, 90, 2, 13, // 600 - picador
		1, // 601 - swatvan
		1, 1, 77, 1, 1, 45, 1, 1, // 602 - alpha
		1, 1, 77, 1, 1, 45, 1, 1, // 603 - phoenix
		76, 76, 76, 76, 76, 76, 76, 88, // 604 - glenshit
		8, 8, 8, 8, 11, 90, 2, 13, // 605 - sadlshit
		// 606 - bagboxa
		// 607 - bagboxb
		1, // 608 - tugstair
		36 // 609 - boxburg
		// 610 - farmtr1
		// 611 - utiltr1
	};

	static const uint8_t colour3Camper[8] = { 0x01, 0x01, 0x03, 0x00, 0x03, 0x03, 0x08, 0x78 };
	static const uint8_t colour4Camper[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const uint8_t colour3Cement[8] = { 0x17, 0x7B, 0x1F, 0x1E, 0x17, 0x3E, 0x40, 0x40 };
	static const uint8_t colour4Cement[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const uint8_t colour3Squalo[8] = { 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01 };
	static const uint8_t colour4Squalo[8] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };

	if (modelid > 611 || modelid < 400)
	{
		colour1 = 127;
		colour2 = 127;
	}
	else
	{
		uint16_t start = vehicleIndexes[modelid - 400];
		uint16_t end = vehicleIndexes[modelid - 399];

		// Choosing a random index in [start, end) interval
		// Ternary operator is used because rand() % 0 is an invalid operation.
		uint16_t index = end == start ? start : (rand() % (end - start) + start);

		colour1 = vehiclePrimaryColours[index];
		colour2 = vehicleSecondaryColours[index];
		switch (modelid)
		{
		case 83:
			colour3 = colour3Camper[index],
			colour4 = colour4Camper[index];
			break;
		case 124:
			colour3 = colour3Cement[index],
			colour4 = colour4Cement[index];
			break;
		case 46:
			colour3 = colour3Squalo[index],
			colour4 = colour4Squalo[index];
			break;
		}
	}
}

inline void getRandomVehicleColour(int modelid, int& colour1, int& colour2)
{
	int colour3, colour4;
	return getRandomVehicleColour(modelid, colour1, colour2, colour3, colour4);
}

inline uint32_t carColourIndexToColour(int index, uint32_t alpha = 0xFF)
{
	static const uint32_t vehicleColours[256] = {
		0x00000000, 0xF5F5F500, 0x2A77A100, 0x84041000, 0x26373900, 0x86446E00, 0xD78E1000, 0x4C75B700,
		0xBDBEC600, 0x5E707200, 0x46597A00, 0x656A7900, 0x5D7E8D00, 0x58595A00, 0xD6DAD600, 0x9CA1A300,
		0x335F3F00, 0x730E1A00, 0x7B0A2A00, 0x9F9D9400, 0x3B4E7800, 0x732E3E00, 0x691E3B00, 0x96918C00,
		0x51545900, 0x3F3E4500, 0xA5A9A700, 0x635C5A00, 0x3D4A6800, 0x97959200, 0x421F2100, 0x5F272B00,
		0x8494AB00, 0x767B7C00, 0x64646400, 0x5A575200, 0x25252700, 0x2D3A3500, 0x93A39600, 0x6D7A8800,
		0x22191800, 0x6F675F00, 0x7C1C2A00, 0x5F0A1500, 0x19382600, 0x5D1B2000, 0x9D987200, 0x7A756000,
		0x98958600, 0xADB0B000, 0x84898800, 0x304F4500, 0x4D626800, 0x16224800, 0x272F4B00, 0x7D625600,
		0x9EA4AB00, 0x9C8D7100, 0x6D182200, 0x4E688100, 0x9C9C9800, 0x91734700, 0x661C2600, 0x949D9F00,
		0xA4A7A500, 0x8E8C4600, 0x341A1E00, 0x6A7A8C00, 0xAAAD8E00, 0xAB988F00, 0x851F2E00, 0x6F829700,
		0x58585300, 0x9AA79000, 0x601A2300, 0x20202C00, 0xA4A09600, 0xAA9D8400, 0x78222B00, 0x0E316D00,
		0x722A3F00, 0x7B715E00, 0x741D2800, 0x1E2E3200, 0x4D322F00, 0x7C1B4400, 0x2E5B2000, 0x395A8300,
		0x6D283700, 0xA7A28F00, 0xAFB1B100, 0x36415500, 0x6D6C6E00, 0x0F6A8900, 0x204B6B00, 0x2B3E5700,
		0x9B9F9D00, 0x6C849500, 0x4D849500, 0xAE9B7F00, 0x406C8F00, 0x1F253B00, 0xAB927600, 0x13457300,
		0x96816C00, 0x64686A00, 0x10508200, 0xA1998300, 0x38569400, 0x52566100, 0x7F695600, 0x8C929A00,
		0x596E8700, 0x47353200, 0x44624F00, 0x730A2700, 0x22345700, 0x640D1B00, 0xA3ADC600, 0x69585300,
		0x9B8B8000, 0x620B1C00, 0x5B5D5E00, 0x62442800, 0x73182700, 0x1B376D00, 0xEC6AAE00, 0x00000000,
		0x17751700, 0x21060600, 0x12547800, 0x452A0D00, 0x571E1E00, 0x01070100, 0x25225A00, 0x2C89AA00,
		0x8A4DBD00, 0x35963A00, 0xB7B7B700, 0x464C8D00, 0x84888C00, 0x81786700, 0x817A2600, 0x6A506F00,
		0x583E6F00, 0x8CB97200, 0x824F7800, 0x6D276A00, 0x1E1D1300, 0x1E130600, 0x1F251800, 0x2C453100,
		0x1E4C9900, 0x2E5F4300, 0x1E994800, 0x1E999900, 0x99997600, 0x7C849900, 0x992E1E00, 0x2C1E0800,
		0x14240700, 0x993E4D00, 0x1E4C9900, 0x19818100, 0x1A292A00, 0x16616F00, 0x1B668700, 0x6C3F9900,
		0x481A0E00, 0x7A739900, 0x746D9900, 0x53387E00, 0x22240700, 0x3E190C00, 0x46210E00, 0x991E1E00,
		0x8D4C8D00, 0x805B8000, 0x7B3E7E00, 0x3C173700, 0x73351700, 0x78181800, 0x83341A00, 0x8E2F1C00,
		0x7E3E5300, 0x7C6D7C00, 0x020C0200, 0x07240700, 0x16301200, 0x16301B00, 0x642B4F00, 0x36845200,
		0x99959000, 0x818D9600, 0x99991E00, 0x7F994C00, 0x83929200, 0x78822200, 0x2B3C9900, 0x3A3A0B00,
		0x8A794E00, 0x0E1F4900, 0x15371C00, 0x15273A00, 0x37577500, 0x06082000, 0x07132600, 0x20394B00,
		0x2C508900, 0x15426C00, 0x10325000, 0x24166300, 0x69201500, 0x8C8D9400, 0x51601300, 0x090F0200,
		0x8C573A00, 0x52888E00, 0x995C5200, 0x99581E00, 0x993A6300, 0x998F4E00, 0x99311E00, 0x0D184200,
		0x521E1E00, 0x42420D00, 0x4C991E00, 0x082A1D00, 0x96821D00, 0x197F1900, 0x3B141F00, 0x74521700,
		0x893F8D00, 0x7E1A6C00, 0x0B370B00, 0x27450D00, 0x071F2400, 0x78457300, 0x8A653A00, 0x73261700,
		0x31949000, 0x56941D00, 0x59163D00, 0x1B8A2F00, 0x38160B00, 0x04180400, 0x355D8E00, 0x2E3F5B00,
		0x561A2800, 0x4E0E2700, 0x706C6700, 0x3B3E4200, 0x2E2D3300, 0x7B7E7D00, 0x4A444200, 0x28344E00
	};
	if (0 <= index && index < 256)
	{
		return vehicleColours[index];
	}
	return 0;
}
}
