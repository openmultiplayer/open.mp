#pragma once

#include "vehicles.hpp"
#include <types.hpp>

namespace Impl
{

/// Returns 0xFF for invalid vehicles, and returns 0 for vehicles with only a driver.
inline uint8_t getVehiclePassengerSeats(int model)
{
	if (!isValidVehicleModel(model))
	{
		return 255;
	}

	static const StaticArray<uint8_t, MAX_VEHICLE_MODELS> allSeats = {
		3, // 400 - landstal
		1, // 401 - bravura
		1, // 402 - buffalo
		1, // 403 - linerun
		3, // 404 - peren
		3, // 405 - sentinel
		0, // 406 - dumper
		1, // 407 - firetruk
		1, // 408 - trash
		3, // 409 - stretch
		1, // 410 - manana
		1, // 411 - infernus
		1, // 412 - voodoo
		3, // 413 - pony
		1, // 414 - mule
		1, // 415 - cheetah
		3, // 416 - ambulan
		1, // 417 - leviathn
		3, // 418 - moonbeam
		1, // 419 - esperant
		3, // 420 - taxi
		3, // 421 - washing
		1, // 422 - bobcat
		1, // 423 - mrwhoop
		1, // 424 - bfinject
		0, // 425 - hunter
		3, // 426 - premier
		3, // 427 - enforcer
		3, // 428 - securica
		1, // 429 - banshee
		0, // 430 - predator
		8, // 431 - bus
		0, // 432 - rhino
		1, // 433 - barracks
		1, // 434 - hotknife
		0, // 435 - artict1
		1, // 436 - previon
		8, // 437 - coach
		3, // 438 - cabbie
		1, // 439 - stallion
		3, // 440 - rumpo
		0, // 441 - rcbandit
		1, // 442 - romero
		1, // 443 - packer
		1, // 444 - monster
		3, // 445 - admiral
		0, // 446 - squalo
		1, // 447 - seaspar
		0, // 448 - pizzaboy
		5, // 449 - tram
		0, // 450 - artict2
		1, // 451 - turismo
		0, // 452 - speeder
		0, // 453 - reefer
		0, // 454 - tropic
		1, // 455 - flatbed
		1, // 456 - yankee
		1, // 457 - caddy
		3, // 458 - solair
		3, // 459 - topfun
		1, // 460 - skimmer
		1, // 461 - pcj600
		1, // 462 - faggio
		1, // 463 - freeway
		1, // 464 - rcbaron
		1, // 465 - rcraider
		3, // 466 - glendale
		3, // 467 - oceanic
		1, // 468 - sanchez
		1, // 469 - sparrow
		3, // 470 - patriot
		1, // 471 - quad
		0, // 472 - coastg
		0, // 473 - dinghy
		1, // 474 - hermes
		1, // 475 - sabre
		0, // 476 - rustler
		1, // 477 - zr350
		1, // 478 - walton
		3, // 479 - regina
		1, // 480 - comet
		0, // 481 - bmx
		3, // 482 - burrito
		2, // 483 - camper
		0, // 484 - marquis
		0, // 485 - baggage
		0, // 486 - dozer
		3, // 487 - maverick
		1, // 488 - vcnmav
		1, // 489 - rancher
		3, // 490 - fbiranch
		1, // 491 - virgo
		3, // 492 - greenwoo
		0, // 493 - jetmax
		1, // 494 - hotring
		1, // 495 - sandking
		1, // 496 - blistac
		3, // 497 - polmav
		3, // 498 - boxville
		1, // 499 - benson
		1, // 500 - mesa
		1, // 501 - rcgoblin
		1, // 502 - hotrina
		1, // 503 - hotrinb
		1, // 504 - bloodra
		1, // 505 - rnchlure
		1, // 506 - supergt
		3, // 507 - elegant
		1, // 508 - journey
		0, // 509 - bike
		0, // 510 - mtbike
		1, // 511 - beagle
		0, // 512 - cropdust
		0, // 513 - stunt
		1, // 514 - petro
		1, // 515 - rdtrain
		3, // 516 - nebula
		1, // 517 - majestic
		1, // 518 - buccanee
		0, // 519 - shamal
		0, // 520 - hydra
		1, // 521 - fcr900
		1, // 522 - nrg500
		1, // 523 - copbike
		1, // 524 - cement
		1, // 525 - towtruck
		1, // 526 - fortune
		1, // 527 - cadrona
		1, // 528 - fbitruck
		3, // 529 - willard
		0, // 530 - forklift
		0, // 531 - tractor
		0, // 532 - combine
		1, // 533 - feltzer
		1, // 534 - remingtn
		1, // 535 - slamvan
		1, // 536 - blade
		5, // 537 - freight
		5, // 538 - streak
		0, // 539 - vortex
		3, // 540 - vincent
		1, // 541 - bullet
		1, // 542 - clover
		1, // 543 - sadler
		1, // 544 - firela
		1, // 545 - hustler
		3, // 546 - intruder
		3, // 547 - primo
		1, // 548 - cargobob
		1, // 549 - tampa
		3, // 550 - sunrise
		3, // 551 - merit
		1, // 552 - utility
		0, // 553 - nevada
		1, // 554 - yosemite
		1, // 555 - windsor
		1, // 556 - monstera
		1, // 557 - monsterb
		1, // 558 - uranus
		1, // 559 - jester
		3, // 560 - sultan
		3, // 561 - stratum
		1, // 562 - elegy
		1, // 563 - raindanc
		0, // 564 - rctiger
		1, // 565 - flash
		3, // 566 - tahoma
		3, // 567 - savanna
		0, // 568 - bandito
		5, // 569 - freiflat
		5, // 570 - streakc
		0, // 571 - kart
		0, // 572 - mower
		1, // 573 - duneride
		0, // 574 - sweeper
		1, // 575 - broadway
		1, // 576 - tornado
		1, // 577 - at400
		1, // 578 - dft30
		3, // 579 - huntley
		3, // 580 - stafford
		1, // 581 - bf400
		3, // 582 - newsvan
		0, // 583 - tug
		0, // 584 - petrotr
		3, // 585 - emperor
		1, // 586 - wayfarer
		1, // 587 - euros
		1, // 588 - hotdog
		1, // 589 - club
		5, // 590 - freibox
		0, // 591 - artict3
		1, // 592 - androm
		1, // 593 - dodo
		1, // 594 - rccam
		0, // 595 - launch
		3, // 596 - copcarla
		3, // 597 - copcarsf
		3, // 598 - copcarvg
		1, // 599 - copcarru
		1, // 600 - picador
		1, // 601 - swatvan
		1, // 602 - alpha
		1, // 603 - phoenix
		3, // 604 - glenshit
		1, // 605 - sadlshit
		0, // 606 - bagboxa
		0, // 607 - bagboxb
		0, // 608 - tugstair
		3, // 609 - boxburg
		0, // 610 - farmtr1
		0 // 611 - utiltr1
	};

	return allSeats[model - 400];
}

}
