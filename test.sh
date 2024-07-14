check_result () {
  ./FindEar $1 > $2
  result=`grep "Total Area" $2 | awk '{print $4}'`
  if [ $result == $3 ]
  then
     echo "Test $1 passed with result $3"
  else
     echo "Test $1 failed with result $3(golden $3)"
  fi
}

check_result test.csv test.log 1
check_result simple_concave_poly.csv simple_concave_poly.log 0.65
check_result concave_poly.csv concave_poly.log 1269.19
