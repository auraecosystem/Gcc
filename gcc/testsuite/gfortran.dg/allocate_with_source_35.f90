! { dg-do run }
!
! PR fortran/128415
! Check correct initialization of the array descriptor element length in the
! context of an ALLOCATE statement whose array spec is implicit, guessed from
! the SOURCE.

program prog
   implicit none
   integer, parameter :: n = 5
   type :: t1
      integer :: c1, c2
   end type
   type(t1) :: a(n)
   integer :: i
   a = [ (t1(3*i+2, 2*i*i+1), i=1,n) ]
   call sub1(a)
contains
   subroutine sub1(x)
      type(t1) :: x(n)
      class(*), pointer :: p(:)
      integer :: i
      allocate(p, source=x%c2)
      call check_logical(is_contiguous(p), .true., 1)
      deallocate(p)
   end subroutine
   subroutine check_logical(x, y, e)
      logical, intent(in) :: x, y
      integer, intent(in) :: e
      !print *, e, (x .eqv. y ? "PASS" : "FAIL"), x, y
      if (x .neqv. y) error stop e
   end subroutine
end program
