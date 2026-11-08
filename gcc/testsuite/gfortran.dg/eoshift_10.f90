! { dg-do run }
!
! PR libfortran/128559
! Check usage of the descriptor span when accessing arrays from the library.
! The EOSHIFT call used to copy the wrong values to its result.

program prog
   implicit none
   integer, parameter :: n = 7
   type :: t1
      integer :: c1, c2
   end type
   type, extends(t1) :: t2
      integer :: c3
   end type
   type(t2), target :: a(n)
   integer :: i
   a = [ (t2(2*i*i+1, 3*i+2, i*i), i=1,n) ]
   call sub(a%t1)
contains
   subroutine sub(x)
      type(t1), pointer, intent(in) :: x(:)
      type(t1) :: fill
      integer :: lshift = 3, i
      fill = t1(-1, -2)
      call check_all(eoshift(x, lshift, fill))
   end subroutine
   subroutine check_all(x)
      type(t1), target, intent(in) :: x(:)
      call check(x%c1, [33, 51, 73, 99, -1, -1, -1], 1)
      call check(x%c2, [14, 17, 20, 23, -2, -2, -2], 2)
   end subroutine
   subroutine check(x, y, e)
      integer, target, intent(in) :: x(:), y(:)
      integer, intent(in) :: e
      if (size(x, 1) /= size(y, 1)) error stop e*10+8
      !do i = 1, n
      !   print *, e*10+i, (x(i) == y(i) ? "PASS" : "FAIL"), x(i), y(i)
      !end do
      if (any(x /= y)) error stop e*10+9
   end subroutine
end program
