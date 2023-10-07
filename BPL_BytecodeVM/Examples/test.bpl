if: 1 == 1 && (2 == 2 && 3 != 3*3 ) {
	print("if");
}
elif: (2 == 2 || 1 == 4) && 3 == 3{
	print("else");
}
else{
	print("elif");
}

print("------");

i = 0;
loop{
	if: i >= 12{
		break;
	}

	print(i);

	i = i+1;

}

c = ?{(1 == 1 && 12==1) || 1 == 2: 1:2};

print(
	#{c: 
		1->2+2*2;
		2-> 2;
		3-> get_number();
	});

fn: get_number(){
	return: 3;
}