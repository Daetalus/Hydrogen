
//
//  Operators
//


#include "operators.h"


//
//  Operator Functions
//

// Returns true if the given token is a binary operator.
bool is_binary_operator(TokenType operator) {
	return operator == TOKEN_ADDITION           ||
		operator == TOKEN_SUBTRACTION           ||
		operator == TOKEN_MULTIPLICATION        ||
		operator == TOKEN_DIVISION              ||
		operator == TOKEN_MODULO                ||

		operator == TOKEN_BOOLEAN_AND           ||
		operator == TOKEN_BOOLEAN_OR            ||
		operator == TOKEN_EQUAL                 ||
		operator == TOKEN_NOT_EQUAL             ||
		operator == TOKEN_LESS_THAN             ||
		operator == TOKEN_LESS_THAN_EQUAL_TO    ||
		operator == TOKEN_GREATER_THAN          ||
		operator == TOKEN_GREATER_THAN_EQUAL_TO ||

		operator == TOKEN_LEFT_SHIFT            ||
		operator == TOKEN_RIGHT_SHIFT           ||
		operator == TOKEN_BITWISE_AND           ||
		operator == TOKEN_BITWISE_OR            ||
		operator == TOKEN_BITWISE_XOR;
}


// Returns the precedence of an operator.
int operator_precedence(TokenType operator) {
	switch(operator) {
	case TOKEN_NEGATION:
	case TOKEN_BITWISE_NOT:
	case TOKEN_BOOLEAN_NOT:
		return 11;

	case TOKEN_MULTIPLICATION:
	case TOKEN_DIVISION:
	case TOKEN_MODULO:
		return 10;

	case TOKEN_ADDITION:
	case TOKEN_SUBTRACTION:
		return 9;

	case TOKEN_LEFT_SHIFT:
	case TOKEN_RIGHT_SHIFT:
		return 8;

	case TOKEN_LESS_THAN:
	case TOKEN_LESS_THAN_EQUAL_TO:
	case TOKEN_GREATER_THAN:
	case TOKEN_GREATER_THAN_EQUAL_TO:
		return 7;

	case TOKEN_EQUAL:
	case TOKEN_NOT_EQUAL:
		return 6;

	case TOKEN_BITWISE_AND:
		return 5;
	case TOKEN_BITWISE_XOR:
		return 4;
	case TOKEN_BITWISE_OR:
		return 3;
	case TOKEN_BOOLEAN_AND:
		return 2;
	case TOKEN_BOOLEAN_OR:
		return 1;

	default:
		return -1;
	}
}


// Returns the associativity of an operator.
Associativity operator_associativity(TokenType operator) {
	if (is_binary_operator(operator)) {
		// No right associative operators at the moment.
		return ASSOCIATIVITY_LEFT;
	}

	return -1;
}



//
//  Mathematical Operators
//

void operator_addition(void) {

}


void operator_subtraction(void) {

}


void operator_multiplication(void) {

}


void operator_division(void) {

}


void operator_modulo(void) {

}


void operator_negation(void) {

}



//
//  Boolean Operators
//

void operator_boolean_and(void) {

}


void operator_boolean_or(void) {

}


void operator_boolean_not(void) {

}


void operator_equal(void) {

}


void operator_not_equal(void) {

}


void operator_less_than(void) {

}


void operator_less_than_equal_to(void) {

}


void operator_greater_than(void) {

}


void operator_greater_than_equal_to(void) {

}



//
//  Bitwise Operators
//

void operator_left_shift(void) {

}


void operator_right_shift(void) {

}


void operator_bitwise_and(void) {

}


void operator_bitwise_or(void) {

}


void operator_bitwise_not(void) {

}


void operator_bitwise_xor(void) {

}
