from cryptography.fernet import Fernet

class Encryptor:
    _key: bytes | None = None
    _fernet: Fernet | None = None

    def set_key(self, key: bytes) -> None:
        self._key = key
        self._fernet = Fernet(key)

    @staticmethod
    def generate_key() -> bytes:
        return Fernet.generate_key()

    def encrypt(self, message: str) -> str:
        if self._fernet is None:
            return message
        return self._fernet.encrypt(message.encode()).decode()

    def decrypt(self, token: str) -> str | None:
        if self._fernet is None:
            return token
        try:
            return self._fernet.decrypt(token.encode()).decode()
        except Exception:
            return None

    def remove_key(self) -> None:
        self._key = None
        self._fernet = None
