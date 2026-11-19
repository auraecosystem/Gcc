! { dg-do run }
!
! PR fortran/128857
! Check that the dynamic type is taken into account when initializing array
! descriptors.  The element length of the temporary descriptor to implement
! the call to IS_CONTIGUOUS intrinsic used to be set to that of T1 instead of
! that of T2, causing IS_CONTIGUOUS to return the wrong result.

program prog
   implicit none
   integer, parameter :: n = 5
   type :: t1
      integer :: c1, c2
   end type
   type, extends(t1) :: t2
      integer :: c3
   end type
   type(t2), target :: a(n)
   integer :: i
   a = [ (t2(1, 2, 3), i=1,n) ]
   call sub(a)
contains
   subroutine sub(x)
      class(t1), target, intent(in) :: x(:)
      if (.not. is_contiguous(x)) error stop 1
   end subroutine
end program

