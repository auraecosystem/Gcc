! { dg-do run }
!
! PR fortran/127612
! Check that pointer assignment correctly sets the span of the array
! descriptor.  The span used to be that of the declarad type for contiguous
! pointers.

program prog
   implicit none
   integer, parameter :: n = 5
   type :: t1
      integer :: c1, c2
   end type
   type, extends(t1) :: t2
      integer :: c3
   end type
   class(t1), allocatable, target :: a(:)
   class(t1), pointer :: p0(:), p1(:)
   integer :: i
   a = [t2 :: (t2(7*i+5,3*i+2,i), i=1,n) ]
   p0(1:n) => a
   p1(1:n) => p0
   call sub(a, [ 12, 19, 26, 33, 40 ], [  5,  8, 11, 14, 17 ], 1)
   a = [t2 :: (t2(3*i*i+2,i*i,i), i=1,n) ]
   call sub(a, [  5, 14, 29, 50, 77 ], [  1,  4,  9, 16, 25 ], 2)
contains
   subroutine sub(x, v1, v2, e)
      class(t1), target, intent(in) :: x(:)
      integer, intent(in) :: v1(:), v2(:), e
      class(t1), contiguous, pointer :: q0(:), q1(:)
      call check(a , v1, v2, e*10+1)
      call check(p0, v1, v2, e*10+2)
      call check(p1, v1, v2, e*10+3)
      call check(x , v1, v2, e*10+6)
      q0(1:n) => x
      call check(q0, v1, v2, e*10+7)
      q1(1:n) => q0
      call check(q1, v1, v2, e*10+8)
   end subroutine
   subroutine check(a, v1, v2, e)
      type(t1), target, intent(in) :: a(:)
      integer, intent(in) :: v1(:), v2(:), e
      call compare(a%c1, v1, e*10+1)
      call compare(a%c2, v2, e*10+2)
   end subroutine
   subroutine compare(a, r, e)
      integer, intent(in) :: a(:), r(:)
      integer, intent(in) :: e
      if (size(a, dim=1) /= size(r, dim=1)) error stop e*10+9
      !do i = 1, size(a,dim=1)
      !   print *, e*10+i, (a(i) == r(i) ? "PASS" : "FAIL"), a(i), r(i)
      !end do
      if (any(a /= r)) error stop e*10+1
   end subroutine
end program
