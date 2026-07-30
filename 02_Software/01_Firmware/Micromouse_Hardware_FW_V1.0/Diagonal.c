#include "Diagonal.h"

////check different Patterns of curves/diagonals to drive Fast
//diagCommand CheckDiagonal(Point shortestPath[gSizeMouseMaze * gSizeMouseMaze], int currentIndex, bool direction){
//	
//		
//	diagCommand diagCom = {0};
//	diagCom.currentIndex = currentIndex;
//	
//	int rememberX = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex-1].x;
//	int rememberY = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex-1].y;
//	
//	
//	//Index bis zur Kurve reduzieren
//	if(direction == 0){
//		while(shortestPath[diagCom.currentIndex].x != shortestPath[diagCom.currentIndex-1].x){
//			diagCom.currentIndex--;
//			if(diagCom.currentIndex <= 0){
//				break;
//			}

//		}
//		direction = 1;
//	}
//	else if (direction == 1){
//		//RHA unebdingt entferne && shortestPath[Index].y != 7
//		while(shortestPath[diagCom.currentIndex].y != shortestPath[diagCom.currentIndex-1].y){
//			diagCom.currentIndex--;
//			if(diagCom.currentIndex <= 0){
//				break;
//			}

//		}
//		direction = 0;
//	}
//	
//	//Falls Ziel schon erreicht
//	if(diagCom.currentIndex <= 0){
//		return diagCom;
//	}
//	
//	
//	
//	//Es gibt vier F�lle rememberX ist -1 oder 1 oder rememberY ist 1 oder -1

//	diagCom.moveDiags = 1;
//	
//	bool lastMoveX = false;
//	bool stopWhile = true;
//	
//	//rigth
//	if(rememberX == 1){
//		//Wert f�r rememberY bestimmen
//		rememberY = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex-1].y;
//		lastMoveX = false;
//		if(rememberY == 1){
//			diagCom.startTurnRight = true;
//		}
//		else if (rememberY == -1){
//			diagCom.startTurnRight = false;
//		}
//		else if(rememberY == 0){
//			//something went wrong should not be 0
//			return diagCom;
//		}
//		while (1) {
//			diagCom.currentIndex--;
//			if (diagCom.currentIndex <= 0) break;

//			int dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
//			int dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;

//			//dx muss mit dem Anfangswert von rememberX �bereinstimmen
//			//ansonsten w�re es eine 180� Kurve
//			if (!lastMoveX && dx == rememberX) {
//					diagCom.moveDiags++;
//					diagCom.isDiag = true;
//					lastMoveX = true;
//					diagCom.EndTurnRight = false;
//			} 
//			else if (lastMoveX && dy == rememberY) {
//					diagCom.moveDiags++;
//					lastMoveX = false;
//					diagCom.EndTurnRight = true;
//			} 
//			else {
//					break;
//			}
//		}
//	}
//	// left
//	else if (rememberX == -1) {
//			rememberY = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;
//			lastMoveX = false;

//			if (rememberY == 1) diagCom.startTurnRight = false;   // nach unten -> Kurve nach links
//			else if (rememberY == -1) diagCom.startTurnRight = true; // nach oben -> Kurve nach rechts
//			else return diagCom;

//			while (1) {
//					if (diagCom.currentIndex <= 1) break;
//					diagCom.currentIndex--;

//					int dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
//					int dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;

//					if (!lastMoveX && dx == rememberX) {
//							diagCom.moveDiags++;
//							diagCom.isDiag = true;
//							lastMoveX = true;
//							diagCom.EndTurnRight = false;
//					} 
//					else if (lastMoveX && dy == rememberY) {
//							diagCom.moveDiags++;
//							lastMoveX = false;
//							diagCom.EndTurnRight = true;
//					} 
//					else {
//							break;
//					}
//			}
//	}

//	// bottom (rememberY == 1)
//	else if (rememberY == 1) {
//			rememberX = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
//			lastMoveX = true;

//			if (rememberX == 1) diagCom.startTurnRight = false; // rechts unten -> Kurve nach links
//			else if (rememberX == -1) diagCom.startTurnRight = true; // links unten -> Kurve nach rechts
//			else return diagCom;

//			while (1) {
//					if (diagCom.currentIndex <= 1) break;
//					diagCom.currentIndex--;

//					int dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
//					int dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;

//					if (lastMoveX && dy == rememberY) {
//							diagCom.moveDiags++;
//							diagCom.isDiag = true;
//							lastMoveX = false;
//							diagCom.EndTurnRight = false;
//					} 
//					else if (!lastMoveX && dx == rememberX) {
//							diagCom.moveDiags++;
//							lastMoveX = true;
//							diagCom.EndTurnRight = true;
//					} 
//					else {
//							break;
//					}
//			}
//	}

//	// top (rememberY == -1)
//	else if (rememberY == -1) {
//			rememberX = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
//			lastMoveX = true;

//			if (rememberX == 1) diagCom.startTurnRight = true; // rechts oben -> Kurve nach rechts
//			else if (rememberX == -1) diagCom.startTurnRight = false; // links oben -> Kurve nach links
//			else return diagCom;

//			while (1) {
//					if (diagCom.currentIndex <= 1) break;
//					diagCom.currentIndex--;

//					int dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
//					int dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;

//					if (lastMoveX && dy == rememberY) {
//							diagCom.moveDiags++;
//							diagCom.isDiag = true;
//							lastMoveX = false;
//							diagCom.EndTurnRight = false;
//					} 
//					else if (!lastMoveX && dx == rememberX) {
//							diagCom.moveDiags++;
//							lastMoveX = true;
//							diagCom.EndTurnRight = true;
//					} 
//					else {
//							break;
//					}
//			}
//	}
//	
//	return diagCom;
//}


//gleiche Funktion wie CheckDiagonal nur mit mehreren Kurventypen
diagCommand CheckCurveType(Point shortestPath[gSizeMouseMaze * gSizeMouseMaze], int currentIndex, bool direction){
		
	volatile diagCommand diagCom = {0};
	diagCom.currentIndex = currentIndex;
	
	volatile int rememberX = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex-1].x;
	volatile int rememberY = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex-1].y;
	
	
	//Index bis zur Kurve reduzieren
	if(direction == 0){
		while(shortestPath[diagCom.currentIndex].x != shortestPath[diagCom.currentIndex-1].x){
			diagCom.currentIndex--;
			if(diagCom.currentIndex <= 0){
				break;
			}

		}
		direction = 1;
	}
	else if (direction == 1){
		while(shortestPath[diagCom.currentIndex].y != shortestPath[diagCom.currentIndex-1].y){
			diagCom.currentIndex--;
			if(diagCom.currentIndex <= 0){
				break;
			}

		}
		direction = 0;
	}
	
	//Falls Ziel schon erreicht
	if(diagCom.currentIndex <= 0){
		return diagCom;
	}
	
	
	
	//Es gibt vier F lle rememberX ist -1 oder 1 oder rememberY ist 1 oder -1

	diagCom.moveDiags = 1;
	
	volatile bool lastMoveX = false;
	volatile bool stopWhile = true;
	
	//rigth
	//TODO: RIGTH sollte passen, es muss noch top,left,bottom angepasst werden
	if(rememberX == 1){
		//Wert f r rememberY bestimmen
		rememberY = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex-1].y;
		lastMoveX = false;
		if(rememberY == 1){
			diagCom.startTurnRight = true;
			diagCom.curveType = CURVE_RIGTH_90_OPENFRONT;
		}
		else if (rememberY == -1){
			diagCom.startTurnRight = false;
			diagCom.curveType = CURVE_LEFT_90_OPENFRONT;
		}
		else if(rememberY == 0){
			//something went wrong should not be 0
			return diagCom;
		}
		
		//Hier return wenn keine dynamischen Kurven verwendet werden sollen
		//return diagCom;
		
		while (1) {
			diagCom.currentIndex--;
			if (diagCom.currentIndex <= 0) break;

			volatile int dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
			volatile int dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;

			//dx muss mit dem Anfangswert von rememberX  bereinstimmen
			//ansonsten waere es eine 180  Kurve
			if (!lastMoveX && dx == rememberX) {
					diagCom.moveDiags++;
					//auswerten, ob es eine oder mehrere diagonalen sind
					if(diagCom.startTurnRight){
						if(diagCom.curveType == DIAGONAL_RIGTH_ONE || diagCom.curveType == DIAGONAL_RIGTH_MORE){
							diagCom.curveType = DIAGONAL_RIGTH_MORE;
						}
						else{
							diagCom.curveType = DIAGONAL_RIGTH_ONE;
						}
					}
					else{
						if(diagCom.curveType == DIAGONAL_LEFT_ONE || diagCom.curveType == DIAGONAL_LEFT_MORE){
							diagCom.curveType = DIAGONAL_LEFT_MORE;
						}
						else{
							diagCom.curveType = DIAGONAL_LEFT_ONE;
						}
						
					}
					lastMoveX = true;
					diagCom.EndTurnRight = false;
			} 
			else if (lastMoveX && dy == rememberY) {
					diagCom.moveDiags++;
					lastMoveX = false;
					diagCom.EndTurnRight = true;
					//auswerten, ob es eine oder mehrere diagonalen sind
					if(diagCom.curveType == DIAGONAL_RIGTH_ONE){
						diagCom.curveType = DIAGONAL_RIGTH_MORE;
					}
					else if(diagCom.curveType == DIAGONAL_LEFT_ONE){
						diagCom.curveType = DIAGONAL_LEFT_MORE;
					}
			} 
			//180  Kurve
			else if(!lastMoveX && dx != rememberX){
				
				//90° Kurve
				if(dx == 0){
					return diagCom;
				}
				
				diagCom.currentIndex--;
				if(diagCom.currentIndex >= 1){
					dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;
					dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
				
					if(diagCom.curveType == CURVE_RIGTH_90_OPENFRONT){
						if(dy == 0){
							diagCom.curveType = CURVE_RIGTH_180_OPENFRONT;
						}
						else{
							diagCom.curveType = CURVE_RIGTH_180_OPENSIDE;
						}

					}
					else if(diagCom.curveType == CURVE_LEFT_90_OPENFRONT){
						if(dy == 0){
							diagCom.curveType = CURVE_LEFT_180_OPENFRONT;
						}
						else{
							diagCom.curveType = CURVE_LEFT_180_OPENSIDE;
						}
					}
				}
				return diagCom; //180  Kurve
			}
		}
	}
	// LEFT
	else if(rememberX == -1){
			// rememberY bestimmen
			rememberY = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex-1].y;
			lastMoveX = false;

			if(rememberY == 1){
					diagCom.startTurnRight = false;
					diagCom.curveType = CURVE_LEFT_90_OPENFRONT;
			}
			else if(rememberY == -1){
					diagCom.startTurnRight = true;
					diagCom.curveType = CURVE_RIGTH_90_OPENFRONT;
			}
			else{
					// Fehlerfall
					return diagCom;
			}
			//Hier return wenn keine dynamischen Kurven verwendet werden sollen
			//return diagCom;

			while(1){
					diagCom.currentIndex--;
					if(diagCom.currentIndex <= 0) break;

					volatile int dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
					volatile int dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;

					// Gleiche Logik wie RIGHT: Prüfen, ob neue Diagonale
					if(!lastMoveX && dx == rememberX){
							diagCom.moveDiags++;

							// Mehrfach-/Einzel-Diagonal bestimmen
							if(diagCom.startTurnRight){
									if(diagCom.curveType == DIAGONAL_RIGTH_ONE || diagCom.curveType == DIAGONAL_RIGTH_MORE)
											diagCom.curveType = DIAGONAL_RIGTH_MORE;
									else
											diagCom.curveType = DIAGONAL_RIGTH_ONE;
							}
							else{
									if(diagCom.curveType == DIAGONAL_LEFT_ONE || diagCom.curveType == DIAGONAL_LEFT_MORE)
											diagCom.curveType = DIAGONAL_LEFT_MORE;
									else
											diagCom.curveType = DIAGONAL_LEFT_ONE;
							}

							lastMoveX = true;
							diagCom.EndTurnRight = false;
					}
					else if(lastMoveX && dy == rememberY){
							diagCom.moveDiags++;
							lastMoveX = false;
							diagCom.EndTurnRight = true;

							// Mehrfach-Diagonal auswerten
							if(diagCom.curveType == DIAGONAL_RIGTH_ONE)
									diagCom.curveType = DIAGONAL_RIGTH_MORE;
							else if(diagCom.curveType == DIAGONAL_LEFT_ONE)
									diagCom.curveType = DIAGONAL_LEFT_MORE;
					}
					// 180° Kurve
					else if(!lastMoveX && dx != rememberX){
						
							//90° Kurve
							if(dx == 0){
								return diagCom;
							}
						
							diagCom.currentIndex--;
							if(diagCom.currentIndex >= 1){
									dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;
									dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;

									if(diagCom.curveType == CURVE_LEFT_90_OPENFRONT){
											if(dy == 0)
													diagCom.curveType = CURVE_LEFT_180_OPENFRONT;
											else
													diagCom.curveType = CURVE_LEFT_180_OPENSIDE;
									}
									else if(diagCom.curveType == CURVE_RIGTH_90_OPENFRONT){
											if(dy == 0)
													diagCom.curveType = CURVE_RIGTH_180_OPENFRONT;
											else
													diagCom.curveType = CURVE_RIGTH_180_OPENSIDE;
									}
							}
							return diagCom; // 180° Kurve
					}
			}
	}


	// BOTTOM (rememberY == 1)
	else if (rememberY == 1) {
			rememberX = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
			lastMoveX = true;

			if (rememberX == 1) {
					diagCom.startTurnRight = false;
					diagCom.curveType = CURVE_LEFT_90_OPENFRONT;
			}
			else if (rememberX == -1) {
					diagCom.startTurnRight = true;
					diagCom.curveType = CURVE_RIGTH_90_OPENFRONT;
			}
			else return diagCom;
			
			//Hier return wenn keine dynamischen Kurven verwendet werden sollen
			//return diagCom;

			while (1) {
					diagCom.currentIndex--;
					if (diagCom.currentIndex <= 0) break;

					volatile int dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
					volatile int dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;

					// vertikale Bewegung zuerst (dy)
					if (lastMoveX && dy == rememberY) {
							diagCom.moveDiags++;

							if(diagCom.startTurnRight){
									if(diagCom.curveType == DIAGONAL_RIGTH_ONE || diagCom.curveType == DIAGONAL_RIGTH_MORE)
											diagCom.curveType = DIAGONAL_RIGTH_MORE;
									else
											diagCom.curveType = DIAGONAL_RIGTH_ONE;
							}
							else{
									if(diagCom.curveType == DIAGONAL_LEFT_ONE || diagCom.curveType == DIAGONAL_LEFT_MORE)
											diagCom.curveType = DIAGONAL_LEFT_MORE;
									else
											diagCom.curveType = DIAGONAL_LEFT_ONE;
							}

							lastMoveX = false;
							diagCom.EndTurnRight = true;
					}
					else if (!lastMoveX && dx == rememberX) {
							diagCom.moveDiags++;
							lastMoveX = true;
							diagCom.EndTurnRight = false;

							// Mehrfach-Diagonale
							if(diagCom.curveType == DIAGONAL_RIGTH_ONE)
									diagCom.curveType = DIAGONAL_RIGTH_MORE;
							else if(diagCom.curveType == DIAGONAL_LEFT_ONE)
									diagCom.curveType = DIAGONAL_LEFT_MORE;
					}
					// 180° Kurve
					else if(lastMoveX && dy != rememberY){
						
							//90° Kurve
							if(dy == 0){
								return diagCom;
							}

							diagCom.currentIndex--;
							if(diagCom.currentIndex >= 1){
									dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
									dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;

									if(diagCom.curveType == CURVE_LEFT_90_OPENFRONT){
											if(dx == 0)
													diagCom.curveType = CURVE_LEFT_180_OPENFRONT;
											else
													diagCom.curveType = CURVE_LEFT_180_OPENSIDE;
									}
									else if(diagCom.curveType == CURVE_RIGTH_90_OPENFRONT){
											if(dx == 0)
													diagCom.curveType = CURVE_RIGTH_180_OPENFRONT;
											else
													diagCom.curveType = CURVE_RIGTH_180_OPENSIDE;
									}
							}
							return diagCom;
					}
			}
	}


		// TOP (rememberY == -1)
	else if (rememberY == -1) {
			rememberX = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
			lastMoveX = true;

			if (rememberX == 1) {
					diagCom.startTurnRight = true;
					diagCom.curveType = CURVE_RIGTH_90_OPENFRONT;
			}
			else if (rememberX == -1) {
					diagCom.startTurnRight = false;
					diagCom.curveType = CURVE_LEFT_90_OPENFRONT;
			}
			else return diagCom;
			
			//Hier return wenn keine dynamischen Kurven verwendet werden sollen
			//return diagCom;

			while (1) {
					diagCom.currentIndex--;
					if (diagCom.currentIndex <= 0) break;

					volatile int dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
					volatile int dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;

					// vertikale Bewegung zuerst (dy)
					if (lastMoveX && dy == rememberY) {
							diagCom.moveDiags++;

							if(diagCom.startTurnRight){
									if(diagCom.curveType == DIAGONAL_RIGTH_ONE || diagCom.curveType == DIAGONAL_RIGTH_MORE)
											diagCom.curveType = DIAGONAL_RIGTH_MORE;
									else
											diagCom.curveType = DIAGONAL_RIGTH_ONE;
							}
							else{
									if(diagCom.curveType == DIAGONAL_LEFT_ONE || diagCom.curveType == DIAGONAL_LEFT_MORE)
											diagCom.curveType = DIAGONAL_LEFT_MORE;
									else
											diagCom.curveType = DIAGONAL_LEFT_ONE;
							}

							lastMoveX = false;
							diagCom.EndTurnRight = true;
					}
					else if (!lastMoveX && dx == rememberX) {
							diagCom.moveDiags++;
							lastMoveX = true;
							diagCom.EndTurnRight = false;

							if(diagCom.curveType == DIAGONAL_RIGTH_ONE)
									diagCom.curveType = DIAGONAL_RIGTH_MORE;
							else if(diagCom.curveType == DIAGONAL_LEFT_ONE)
									diagCom.curveType = DIAGONAL_LEFT_MORE;
					}
					// 180° Kurve
					else if(lastMoveX && dy != rememberY){
										
							//90° Kurve
							if(dy == 0){
								return diagCom;
							}
						
							diagCom.currentIndex--;
							if(diagCom.currentIndex >= 1){
									dx = shortestPath[diagCom.currentIndex].x - shortestPath[diagCom.currentIndex - 1].x;
									dy = shortestPath[diagCom.currentIndex].y - shortestPath[diagCom.currentIndex - 1].y;

									if(diagCom.curveType == CURVE_LEFT_90_OPENFRONT){
											if(dx == 0)
													diagCom.curveType = CURVE_LEFT_180_OPENFRONT;
											else
													diagCom.curveType = CURVE_LEFT_180_OPENSIDE;
									}
									else if(diagCom.curveType == CURVE_RIGTH_90_OPENFRONT){
											if(dx == 0)
													diagCom.curveType = CURVE_RIGTH_180_OPENFRONT;
											else
													diagCom.curveType = CURVE_RIGTH_180_OPENSIDE;
									}
							}
							return diagCom;
					}
					else{
						return diagCom;
					}
			}
	}

	return diagCom;
}
